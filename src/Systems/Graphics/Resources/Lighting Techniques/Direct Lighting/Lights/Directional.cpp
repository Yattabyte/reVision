#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Lights\Directional.h"
#include "Engine.h"
#include <minmax.h>


Directional_Tech::~Directional_Tech()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	m_engine->removePrefCallback(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, this);
}

Directional_Tech::Directional_Tech(Engine * engine, Light_Buffers * lightBuffers)
{
	// Default Parameters
	m_engine = engine;
	m_lightSSBO = &lightBuffers->m_lightDirSSBO;
	m_size = 0;
	m_sizeGI = 0;

	// Asset Loading
	m_engine->createAsset(m_shader_CullDynamic, string("Base Lights\\Directional\\Culling_Dynamic"), true);
	m_engine->createAsset(m_shader_CullStatic, string("Base Lights\\Directional\\Culling_Static"), true);
	m_engine->createAsset(m_shader_ShadowDynamic, string("Base Lights\\Directional\\Shadow_Dynamic"), true);
	m_engine->createAsset(m_shader_ShadowStatic, string("Base Lights\\Directional\\Shadow_Static"), true);
	m_engine->createAsset(m_shader_Lighting, string("Base Lights\\Directional\\Light"), true);
	m_engine->createAsset(m_shader_Bounce, string("Base Lights\\Directional\\Bounce"), true);
	engine->createAsset(m_shapeQuad, string("quad"));

	// Primitive Construction
	m_quadVAOLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_indirectShape = StaticBuffer(sizeof(GLuint) * 4, 0);
	m_shapeQuad->addCallback(this, [&]() mutable {
		m_quadVAOLoaded = true;
		m_shapeQuad->updateVAO(m_quadVAO);
		const GLuint data = m_shapeQuad->getSize();
		m_indirectShape.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
	});

	// Initialize Shadows
	m_shadowSize.x = m_engine->addPrefCallback(PreferenceState::C_SHADOW_SIZE_DIRECTIONAL, this, [&](const float &f) { setSize(f); }	);
	m_shadowSize = vec2(max(1.0f, m_shadowSize.x));
	m_shadowCount = 0;
	m_shadowFBO = 0;
	m_shadowDepth = 0;
	m_shadowWNormal = 0;
	m_shadowRFlux = 0;
	glCreateFramebuffers(1, &m_shadowFBO);
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_shadowDepth);
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_shadowWNormal);
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_shadowRFlux);

	glTextureImage3DEXT(m_shadowDepth, GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_shadowSize.x, m_shadowSize.y, 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTextureParameteri(m_shadowDepth, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_shadowDepth, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_shadowDepth, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_shadowDepth, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_shadowDepth, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glNamedFramebufferTexture(m_shadowFBO, GL_DEPTH_ATTACHMENT, m_shadowDepth, 0);

	// Create the World Normal buffer
	glTextureImage3DEXT(m_shadowWNormal, GL_TEXTURE_2D_ARRAY, 0, GL_RGB16F, m_shadowSize.x, m_shadowSize.y, 4, 0, GL_RGB, GL_FLOAT, NULL);
	glTextureParameteri(m_shadowWNormal, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_shadowWNormal, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_shadowWNormal, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_shadowWNormal, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glNamedFramebufferTexture(m_shadowFBO, GL_COLOR_ATTACHMENT0, m_shadowWNormal, 0);
 
	// Create the Radiant Flux buffer
	glTextureImage3DEXT(m_shadowRFlux, GL_TEXTURE_2D_ARRAY, 0, GL_RGB16F, m_shadowSize.x, m_shadowSize.y, 4, 0, GL_RGB, GL_FLOAT, NULL);
	glTextureParameteri(m_shadowRFlux, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_shadowRFlux, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_shadowRFlux, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_shadowRFlux, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glNamedFramebufferTexture(m_shadowFBO, GL_COLOR_ATTACHMENT1, m_shadowRFlux, 0);

	const GLenum Buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glNamedFramebufferDrawBuffers(m_shadowFBO, 2, Buffers);

	// Error Checking
	const GLenum Status = glCheckNamedFramebufferStatus(m_shadowFBO, GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) 
		m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Directional light  Technique", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));

	// Light Bounce Initialization
	GLuint firstBounceData[4] = { 6, 0, 0, 0 }; // count, primCount, first, reserved
	m_indirectBounce = StaticBuffer(sizeof(GLuint) * 4, firstBounceData);
}

vec2 Directional_Tech::getSize() const
{
	return m_shadowSize;
}

void Directional_Tech::registerShadowCaster(int & array_spot)
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
	glTextureImage3DEXT(m_shadowDepth, GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTextureImage3DEXT(m_shadowWNormal, GL_TEXTURE_2D_ARRAY, 0, GL_RGB16F, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 4, 0, GL_RGB, GL_FLOAT, NULL);
	glTextureImage3DEXT(m_shadowRFlux, GL_TEXTURE_2D_ARRAY, 0, GL_RGB16F, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 4, 0, GL_RGB, GL_FLOAT, NULL);
}

void Directional_Tech::unregisterShadowCaster(int & array_spot)
{
	bool found = false;
	for (int x = 0, size = m_freedShadowSpots.size(); x < size; ++x)
		if (m_freedShadowSpots[x] == array_spot)
			found = true;
	if (!found)
		m_freedShadowSpots.push_back(array_spot);
}

void Directional_Tech::updateData(const Visibility_Token & vis_token, const int & updateQuality, const vec3 & camPos)
{	
	m_size = vis_token.specificSize("Light_Directional");
	if (m_size && m_quadVAOLoaded) {
		// Retrieve a sorted list of most important lights to run shadow calc for.
		PriorityLightList queue(updateQuality, camPos);

		for each (const auto &component in vis_token.getTypeList<Lighting_Component>("Light_Directional"))
			queue.insert(component);

		m_queue = queue.toList();
		for each (const auto &c in m_queue)
			c->update(CAM_GEOMETRY_DYNAMIC);

		for each (const auto &c in m_queue)
			c->update(CAM_GEOMETRY_STATIC);

		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &m_size); // update primCount (2nd param)
	}
}

void Directional_Tech::updateDataGI(const Visibility_Token & vis_token, const unsigned int & bounceResolution)
{
	m_sizeGI = vis_token.specificSize("Light_Directional");
	if (m_sizeGI && m_shader_Bounce->existsYet()) {
		const GLuint dirDraws = bounceResolution * m_sizeGI;
		m_indirectBounce.write(sizeof(GLuint), sizeof(GLuint), &dirDraws);
		m_shader_Bounce->Set_Uniform(0, (int)m_sizeGI);
	}
}

void Directional_Tech::renderOcclusionCulling()
{
	if (m_size && m_shader_CullDynamic->existsYet()) {
		// Cull dynamic geometry
		m_shader_CullDynamic->bind();
		glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadowFBO);
		glNamedFramebufferDrawBuffer(m_shadowFBO, GL_NONE);
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
		for each (const auto &c in m_queue)
			c->occlusionPass(CAM_GEOMETRY_DYNAMIC);
		if (m_shader_CullStatic->existsYet()) {
			// Cull static geometry
			m_shader_CullStatic->bind();
			for each (auto & c in m_queue)
				c->occlusionPass(CAM_GEOMETRY_STATIC);
		}		
	}
}

void Directional_Tech::renderShadows()
{
	if (m_size && m_shader_ShadowDynamic->existsYet()) {
		// Render dynamic geometry
		m_shader_ShadowDynamic->bind();
		glViewport(0, 0, m_shadowSize.x, m_shadowSize.y);
		GLenum Buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glNamedFramebufferDrawBuffers(m_shadowFBO, 3, Buffers);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_shadowFBO);
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
		for each (auto &component in m_queue)
			component->shadowPass(CAM_GEOMETRY_DYNAMIC);
		if (m_shader_ShadowStatic->existsYet()) {
			// Render static geometry
			m_shader_ShadowStatic->bind();
			for each (auto &component in m_queue)
				component->shadowPass(CAM_GEOMETRY_STATIC);
		}
	}
}

void Directional_Tech::renderLightBounce()
{
	if (m_sizeGI && m_shader_Bounce->existsYet()) {
		glBindTextureUnit(0, m_shadowDepth);
		glBindTextureUnit(1, m_shadowWNormal);
		glBindTextureUnit(2, m_shadowRFlux);
		m_shader_Bounce->bind();
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
		m_indirectBounce.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
}

void Directional_Tech::renderLighting()
{
	if (m_size && m_shader_Lighting->existsYet() && m_quadVAOLoaded) {
		m_shader_Lighting->bind();										// Shader (directional)
		glBindTextureUnit(4, m_shadowDepth);							// Shadow maps (large maps)
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);		// SSBO light attribute array (directional)
		m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_quadVAO);									// Quad VAO
		glDrawArraysIndirect(GL_TRIANGLES, 0);							// Now draw
	}
}

void Directional_Tech::clearShadow(const int & layer)
{
	const float clearDepth(1.0f);
	const vec3 clear(0.0f);
	glClearTexSubImage(m_shadowDepth, 0, 0, 0, layer * 4, m_shadowSize.x, m_shadowSize.y, 4, GL_DEPTH_COMPONENT, GL_FLOAT, &clearDepth);
	glClearTexSubImage(m_shadowWNormal, 0, 0, 0, layer * 4, m_shadowSize.x, m_shadowSize.y, 4, GL_RGB, GL_FLOAT, &clear);
	glClearTexSubImage(m_shadowRFlux, 0, 0, 0, layer * 4, m_shadowSize.x, m_shadowSize.y, 4, GL_RGB, GL_FLOAT, &clear);
}

void Directional_Tech::setSize(const float & size)
{
	m_shadowSize = vec2(max(size, 1));

	glTextureImage3DEXT(m_shadowDepth, GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glNamedFramebufferTexture(m_shadowFBO, GL_DEPTH_ATTACHMENT, m_shadowDepth, 0);

	glTextureImage3DEXT(m_shadowWNormal, GL_TEXTURE_2D_ARRAY, 0, GL_RGB16F, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 4, 0, GL_RGB, GL_FLOAT, NULL);
	glNamedFramebufferTexture(m_shadowFBO, GL_COLOR_ATTACHMENT0, m_shadowWNormal, 0);

	glTextureImage3DEXT(m_shadowRFlux, GL_TEXTURE_2D_ARRAY, 0, GL_RGB16F, m_shadowSize.x, m_shadowSize.y, m_shadowCount * 4, 0, GL_RGB, GL_FLOAT, NULL);
	glNamedFramebufferTexture(m_shadowFBO, GL_COLOR_ATTACHMENT1, m_shadowRFlux, 0);
}
