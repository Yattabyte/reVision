#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Lights\Spot.h"
#include "Systems\World\World.h"
#include "Managers\Message_Manager.h"
#include "Utilities\EnginePackage.h"
#include <minmax.h>

Spot_Tech::~Spot_Tech()
{
	if (m_shapeCone.get()) m_shapeCone->removeCallback(this);
	m_enginePackage->removePrefCallback(PreferenceState::C_SHADOW_SIZE_SPOT, this);
}

Spot_Tech::Spot_Tech(EnginePackage * enginePackage, Light_Buffers * lightBuffers)
{
	m_enginePackage = enginePackage;
	m_lightSSBO = &lightBuffers->m_lightSpotSSBO;
	m_size = 0;
	m_sizeGI = 0;

	Asset_Shader::Create(m_shader_Lighting, "Base Lights\\Spot\\Light");
	Asset_Shader::Create(m_shader_CullDynamic, "Base Lights\\Spot\\Culling_Dynamic");
	Asset_Shader::Create(m_shader_CullStatic, "Base Lights\\Spot\\Culling_Static");
	Asset_Shader::Create(m_shader_ShadowDynamic, "Base Lights\\Spot\\Shadow_Dynamic");
	Asset_Shader::Create(m_shader_ShadowStatic, "Base Lights\\Spot\\Shadow_Static");
	Asset_Shader::Create(m_shader_Bounce, "Base Lights\\Spot\\Bounce");

	// Primitive Loading
	Asset_Primitive::Create(m_shapeCone, "cone");
	m_coneVAOLoaded = false;
	m_coneVAO = Asset_Primitive::Generate_VAO();
	m_shapeCone->addCallback(this, [&]() {
		m_shapeCone->updateVAO(m_coneVAO);
		m_coneVAOLoaded = true;
		GLuint data[4] = { m_shapeCone->getSize(), 0, 0, 0 }; // count, primCount, first, reserved
		m_indirectShape = StaticBuffer(sizeof(GLuint) * 4, data);
	});

	m_regenSShadows = false;
	m_enginePackage->getSubSystem<System_World>("World")->notifyWhenLoaded(&m_regenSShadows);

	// Initialize Shadows
	m_shadowSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_SHADOW_SIZE_SPOT, this, [&](const float &f) {setSize(f); });
	m_shadowSize = vec2(max(1.0f, m_shadowSize.x));
	m_shadowCount = 0;
	for (int x = 0; x < 2; ++x) {
		m_shadowFBO[x] = 0;
		m_shadowDepth[x] = 0;
		m_shadowWNormal[x] = 0;
		m_shadowRFlux[x] = 0;
		glCreateFramebuffers(1, &m_shadowFBO[x]);
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_shadowDepth[x]);
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_shadowWNormal[x]);
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_shadowRFlux[x]);

		glTextureImage3DEXT(m_shadowDepth[x], GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_shadowSize.x, m_shadowSize.y, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTextureParameteri(m_shadowDepth[x], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_shadowDepth[x], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_shadowDepth[x], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_shadowDepth[x], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_shadowDepth[x], GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_DEPTH_ATTACHMENT, m_shadowDepth[x], 0);

		// Create the World Normal buffer
		glTextureImage3DEXT(m_shadowWNormal[x], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_shadowSize.x, m_shadowSize.y, 1, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureParameteri(m_shadowWNormal[x], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_shadowWNormal[x], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_shadowWNormal[x], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_shadowWNormal[x], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_COLOR_ATTACHMENT0, m_shadowWNormal[x], 0);

		// Create the Radiant Flux buffer
		glTextureImage3DEXT(m_shadowRFlux[x], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_shadowSize.x, m_shadowSize.y, 1, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureParameteri(m_shadowRFlux[x], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_shadowRFlux[x], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_shadowRFlux[x], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_shadowRFlux[x], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_COLOR_ATTACHMENT1, m_shadowRFlux[x], 0);

		GLenum Buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glNamedFramebufferDrawBuffers(m_shadowFBO[x], 2, Buffers);

		GLenum Status = glCheckNamedFramebufferStatus(m_shadowFBO[x], GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) 
			MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "Spot light Technique", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));		
	}

	// Light Bounce Initialization
	GLuint firstBounceData[4] = { 6, 0, 0, 0 }; // count, primCount, first, reserved
	m_indirectBounce = StaticBuffer(sizeof(GLuint) * 4, firstBounceData);
}

vec2 Spot_Tech::getSize() const
{
	return m_shadowSize;
}

void Spot_Tech::registerShadowCaster(int & array_spot)
{
	if (m_freedShadowSpots.size()) {
		array_spot = m_freedShadowSpots.front();
		m_freedShadowSpots.pop_front();
	}
	else {
		array_spot = m_shadowCount;
		m_shadowCount++;
	}

	// Adjust the layer count every time a new light is added (preserve memory rather than preallocating memory for shadows that don't exist)
	for (int x = 0; x < 2; ++x) {
		glTextureImage3DEXT(m_shadowDepth[x], GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_shadowSize.x, m_shadowSize.y, m_shadowCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTextureImage3DEXT(m_shadowWNormal[x], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_shadowSize.x, m_shadowSize.y, m_shadowCount, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureImage3DEXT(m_shadowRFlux[x], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_shadowSize.x, m_shadowSize.y, m_shadowCount, 0, GL_RGB, GL_FLOAT, NULL);
	}
}

void Spot_Tech::unregisterShadowCaster(int & array_spot)
{
	bool found = false;
	for (int x = 0, size = m_freedShadowSpots.size(); x < size; ++x)
		if (m_freedShadowSpots[x] == array_spot)
			found = true;
	if (!found)
		m_freedShadowSpots.push_back(array_spot);
}

void Spot_Tech::updateData(const Visibility_Token & vis_token, const int & updateQuality, const vec3 & camPos)
{	
	m_size = vis_token.specificSize("Light_Spot");
	if (m_size && m_coneVAOLoaded) {
		// Retrieve a sorted list of most important lights to run shadow calc for.
		PriorityLightList queue(updateQuality, camPos);
		m_lightList = vis_token.getTypeList<Lighting_Component>("Light_Spot");

		for each (const auto &component in m_lightList)
			queue.insert(component);

		m_queue = queue.toList();
		for each (const auto &c in m_queue)
			c->update(CAM_GEOMETRY_DYNAMIC);

		if (m_regenSShadows)
			for each (const auto &c in m_lightList)
				c->update(CAM_GEOMETRY_STATIC);

		vector<GLuint> visArray(m_size);
		unsigned int count = 0;
		for each (const auto &component in m_lightList)
			visArray[count++] = component->getBufferIndex();
		m_visShapes.write(0, sizeof(GLuint)*visArray.size(), visArray.data());
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &m_size);
	}
}

void Spot_Tech::updateDataGI(const Visibility_Token & vis_token, const unsigned int & bounceResolution)
{
	m_sizeGI = vis_token.specificSize("Light_Spot");
	if (m_sizeGI && m_shader_Bounce->existsYet()) {
		const GLuint spotDraws = bounceResolution * m_sizeGI;
		vector<GLuint> visArray(m_sizeGI);
		unsigned int count = 0;
		for each (const auto &component in vis_token.getTypeList<Lighting_Component>("Light_Spot"))
			visArray[count++] = component->getBufferIndex();
		m_visSpots.write(0, sizeof(GLuint)*visArray.size(), visArray.data());
		m_indirectBounce.write(sizeof(GLuint), sizeof(GLuint), &spotDraws);
		m_shader_Bounce->Set_Uniform(0, (int)m_sizeGI);
	}
}

void Spot_Tech::renderOcclusionCulling()
{
	if (m_size && m_shader_CullDynamic->existsYet()) {
		// Cull dynamic geometry
		m_shader_CullDynamic->bind();
		glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadowFBO[0]);
		glNamedFramebufferDrawBuffer(m_shadowFBO[0], GL_NONE);
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
		for each (const auto &c in m_queue)
			c->occlusionPass(CAM_GEOMETRY_DYNAMIC);
		if (m_regenSShadows && m_shader_CullStatic->existsYet()) {
			// Cull static geometry
			m_shader_CullStatic->bind();
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadowFBO[1]);
			glNamedFramebufferDrawBuffer(m_shadowFBO[1], GL_NONE);
			for each (auto & c in m_lightList)
				c->occlusionPass(CAM_GEOMETRY_STATIC);
		}
	}
}

void Spot_Tech::renderShadows()
{
	if (m_size && m_shader_ShadowDynamic->existsYet()) {
		// Render dynamic geometry
		m_shader_ShadowDynamic->bind();
		glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
		GLenum Buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glNamedFramebufferDrawBuffers(m_shadowFBO[0], 3, Buffers);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadowFBO[0]);
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
		for each (auto &component in m_queue)
			component->shadowPass(CAM_GEOMETRY_DYNAMIC);
		if (m_regenSShadows && m_shader_ShadowStatic->existsYet()) {
			m_regenSShadows = false;
			// Render static geometry
			m_shader_ShadowStatic->bind();
			glNamedFramebufferDrawBuffers(m_shadowFBO[1], 3, Buffers);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadowFBO[1]);
			for each (auto &component in m_lightList)
				component->shadowPass(CAM_GEOMETRY_STATIC);
		}
	}
}

void Spot_Tech::renderLightBounce()
{
	if (m_sizeGI && m_shader_Bounce->existsYet()) {
		glBindTextureUnit(0, m_shadowDepth[0]);
		glBindTextureUnit(1, m_shadowWNormal[0]);
		glBindTextureUnit(2, m_shadowRFlux[0]);
		glBindTextureUnit(3, m_shadowDepth[1]);
		glBindTextureUnit(4, m_shadowWNormal[1]);
		glBindTextureUnit(5, m_shadowRFlux[1]);
		m_shader_Bounce->bind();
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
		m_visSpots.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_indirectBounce.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
}

void Spot_Tech::renderLighting()
{
	if (m_size && m_shader_Lighting->existsYet() && m_coneVAOLoaded) {
		glEnable(GL_STENCIL_TEST);
		glCullFace(GL_FRONT);

		m_shader_Lighting->bind();										// Shader (spots)
		glBindTextureUnit(4, m_shadowDepth[0]);							// Shadow maps (dynamic)
		glBindTextureUnit(5, m_shadowDepth[1]);							// Shadow maps (static)
		m_visShapes.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);		// SSBO light attribute array (spots)
		m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_coneVAO);									// Cone VAO

		// Draw only into depth-stencil buffer
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0, 0);
		m_shader_Lighting->Set_Uniform(0, true);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Now draw into color buffers
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		m_shader_Lighting->Set_Uniform(0, false);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
}

void Spot_Tech::BindForReading_GI(const GLuint & ShaderTextureUnit)
{
	glBindTextureUnit(ShaderTextureUnit, m_shadowDepth[0]);
	glBindTextureUnit(ShaderTextureUnit + 1, m_shadowWNormal[0]);
	glBindTextureUnit(ShaderTextureUnit + 2, m_shadowRFlux[0]);
}

void Spot_Tech::clearShadow(const unsigned int & type, const int & layer)
{
	const float clearDepth(1.0f);
	const vec3 clear(0.0f);
	glClearTexSubImage(m_shadowDepth[type], 0, 0, 0, layer, m_shadowSize.x, m_shadowSize.y, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
	glClearTexSubImage(m_shadowWNormal[type], 0, 0, 0, layer, m_shadowSize.x, m_shadowSize.y, 1, GL_RGB, GL_FLOAT, &clear);
	glClearTexSubImage(m_shadowRFlux[type], 0, 0, 0, layer, m_shadowSize.x, m_shadowSize.y, 1, GL_RGB, GL_FLOAT, &clear);
}

void Spot_Tech::setSize(const float & size)
{
	m_shadowSize = vec2(max(size, 1));

	for (int x = 0; x < 2; ++x) {
		glTextureImage3DEXT(m_shadowDepth[x], GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_shadowSize.x, m_shadowSize.y, m_shadowCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_DEPTH_ATTACHMENT, m_shadowDepth[x], 0);

		glTextureImage3DEXT(m_shadowWNormal[x], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_shadowSize.x, m_shadowSize.y, m_shadowCount, 0, GL_RGB, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_COLOR_ATTACHMENT0, m_shadowWNormal[x], 0);

		glTextureImage3DEXT(m_shadowRFlux[x], GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, m_shadowSize.x, m_shadowSize.y, m_shadowCount, 0, GL_RGB, GL_FLOAT, NULL);
		glNamedFramebufferTexture(m_shadowFBO[x], GL_COLOR_ATTACHMENT1, m_shadowRFlux[x], 0);
		m_regenSShadows = true;
	}
}
