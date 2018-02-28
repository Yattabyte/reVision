#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Lighting Techniques\DirectLighting_Tech.h"
#include "Systems\Graphics\Lighting Techniques\IndirectDiffuse_GI_Tech.h"
#include "Systems\Graphics\Lighting Techniques\IndirectSpecular_SSR_Tech.h"
#include "Systems\Graphics\FX Techniques\Bloom_Tech.h"
#include "Systems\Graphics\FX Techniques\HDR_Tech.h"
#include "Systems\Graphics\FX Techniques\FXAA_Tech.h"
#include "Utilities\EnginePackage.h"
#include "Systems\World\Camera.h"
#include "Systems\World\ECS\Components\Geometry_Component.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include <random>
#include <minmax.h>


struct Primitive_Observer : Asset_Observer
{
	Primitive_Observer(Shared_Asset_Primitive & asset, const GLuint vao) : Asset_Observer(asset.get()), m_vao_id(vao) {};
	virtual void Notify_Finalized() {
		if (m_asset->existsYet())
			dynamic_pointer_cast<Asset_Primitive>(m_asset)->updateVAO(m_vao_id);
	}
	GLuint m_vao_id;
};

System_Graphics::~System_Graphics()
{
	if (!m_Initialized) {
		m_enginePackage->removePrefCallback(PreferenceState::C_SSAO, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_SSAO_SAMPLES, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_SSAO_BLUR_STRENGTH, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_SSAO_RADIUS, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_SHADOW_QUALITY, this);

		glDeleteBuffers(1, &m_attribID);
		delete m_QuadObserver;		
		for each (auto * tech in m_lightingTechs)
			delete tech;
		for each (auto * tech in m_fxTechs)
			delete tech;
	}
}

System_Graphics::System_Graphics() :
	m_visualFX(), m_gBuffer(), m_lBuffer()
{
	m_quadVAO = 0;
	m_attribID = 0;
	m_renderSize = vec2(1);
}

void System_Graphics::initialize(EnginePackage * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		Asset_Loader::load_asset(m_shaderGeometry, "Geometry\\geometry");
		Asset_Loader::load_asset(m_shaderDirectional_Shadow, "Geometry\\geometryShadowDir");
		Asset_Loader::load_asset(m_shaderPoint_Shadow, "Geometry\\geometryShadowPoint");
		Asset_Loader::load_asset(m_shaderSpot_Shadow, "Geometry\\geometryShadowSpot");
		Asset_Loader::load_asset(m_shapeQuad, "quad");
		Asset_Loader::load_asset(m_shaderSky, "skybox");
		Asset_Loader::load_asset(m_textureSky, "sky\\");
		m_quadVAO = Asset_Primitive::Generate_VAO();
		m_QuadObserver = (void*)(new Primitive_Observer(m_shapeQuad, m_quadVAO));
		m_gBuffer.end();

		m_attribs.m_ssao = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO, this, [&](const float &f) {setSSAO(f); });
		m_attribs.m_aa_samples = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO_SAMPLES, this, [&](const float &f) {setSSAOSamples(f); });
		m_attribs.m_ssao_strength = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO_BLUR_STRENGTH, this, [&](const float &f) {setSSAOStrength(f); });
		m_attribs.m_ssao_radius = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO_RADIUS, this, [&](const float &f) {setSSAORadius(f); });
		m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(vec2(f, m_renderSize.y)); });
		m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(vec2(m_renderSize.x, f)); });
		m_updateQuality = m_enginePackage->addPrefCallback(PreferenceState::C_SHADOW_QUALITY, this, [&](const float &f) {setShadowUpdateQuality(f); });

		glGenBuffers(1, &m_attribID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_attribID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_attribs), &m_attribs, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_attribID);
		GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
		memcpy(p, &m_attribs, sizeof(m_attribs));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		generateKernal();

		m_visualFX.initialize(m_enginePackage);
		m_gBuffer.initialize(m_renderSize, &m_visualFX);
		m_lBuffer.initialize(m_renderSize, m_gBuffer.m_depth_stencil);
		m_shadowBuffer.initialize(enginePackage);

		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

		m_lightingTechs.push_back(new DirectLighting_Tech(&m_gBuffer, &m_lBuffer, &m_shadowBuffer));
		m_lightingTechs.push_back(new IndirectDiffuse_GI_Tech(m_enginePackage, &m_gBuffer, &m_lBuffer, &m_shadowBuffer)); 
		m_lightingTechs.push_back(new IndirectSpecular_SSR_Tech(m_enginePackage, &m_gBuffer, &m_lBuffer, &m_visualFX));
		m_fxTechs.push_back(new Bloom_Tech(enginePackage, &m_lBuffer, &m_visualFX));
		m_fxTechs.push_back(new HDR_Tech(enginePackage));
		m_fxTechs.push_back(new FXAA_Tech());

		m_Initialized = true;
	}
}

void System_Graphics::update(const float & deltaTime)
{
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);

	shared_lock<shared_mutex> read_guard(m_enginePackage->m_Camera.getDataMutex());
	Visibility_Token &vis_token = m_enginePackage->m_Camera.GetVisibilityToken();

	if (m_Initialized &&
		vis_token.size() &&
		m_shapeQuad->existsYet() &&
		m_textureSky->existsYet() &&
		m_shaderSky->existsYet() &&


		m_shaderGeometry->existsYet())
	{
		// Regeneration Phase
		shadowPass(vis_token);
		for each (auto *tech in m_lightingTechs)
			tech->updateLighting(vis_token);

		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
		m_gBuffer.clear();
		m_lBuffer.clear();
		geometryPass(vis_token);
		
		// Writing to lighting fbo
		skyPass();
		for each (auto *tech in m_lightingTechs)
			tech->applyLighting(vis_token);

		// Chain of post-processing effects ending in default fbo
		m_lBuffer.bindForReading();
		for each (auto *tech in m_fxTechs) {
			tech->applyEffect();
			tech->bindForReading();
		}

		m_lBuffer.end();
		m_gBuffer.end();
	}
}

void System_Graphics::setSSAO(const bool & ssao)
{
	m_attribs.m_ssao = ssao;
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics::setSSAOSamples(const int & samples)
{
	m_attribs.m_aa_samples = samples;
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics::setSSAOStrength(const int & strength)
{
	m_attribs.m_ssao_strength = strength;
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics::setSSAORadius(const float & radius)
{
	m_attribs.m_ssao_radius = radius;
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics::resize(const vec2 & size)
{
	m_renderSize = size;
	m_gBuffer.resize(size);
	m_lBuffer.resize(size);
}

void System_Graphics::setShadowUpdateQuality(const float & quality)
{
	m_updateQuality = quality;
}

Shadow_Buffer & System_Graphics::getShadowBuffer()
{
	return m_shadowBuffer;
}

void System_Graphics::generateKernal()
{
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
	std::default_random_engine generator;
	for (int i = 0, t = 0; i < MAX_KERNEL_SIZE; i++, t++) {
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		GLfloat scale = GLfloat(i) / (GLfloat)(MAX_KERNEL_SIZE);
		scale = 0.1f + (scale*scale) * (1.0f - 0.1f);
		sample *= scale;
		m_attribs.kernel[t] = vec4(sample, 1);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics::shadowPass(const Visibility_Token & vis_token)
{
	struct PriorityLightList {
		// Nested Element struct
		struct LightElement {
			double m_updateTime;
			float m_importance;
			Lighting_Component *m_ptr;
			const char * m_lightType;
			LightElement(const double & time, const float & importance, Lighting_Component * ptr,const char * type) {
				m_updateTime = time;
				m_importance = importance;
				m_ptr = ptr;
				m_lightType = type;
			}
		};

		// Constructor
		PriorityLightList(const int &capacity, const vec3 & position) {
			m_capacity = capacity;
			m_position = position;
			m_list.reserve(capacity);
		}

		// Accessors/Updaters
		void add(Lighting_Component * c, const char * type) {
			const size_t &listSize = m_list.size();
			double time = c->getShadowUpdateTime(); 
			int insertionIndex = listSize;	
			for (int x = 0; x < listSize; ++x) {
				if (time < m_list[x].m_updateTime) {
					insertionIndex = x;
					break;
				}
			}
			m_list.insert(m_list.begin() + insertionIndex, LightElement(time, c->getImportance(m_position), c, type));
			if (m_list.size() > m_capacity) {
				m_list.reserve(m_capacity);
				m_list.shrink_to_fit();
			}		
		}
		Visibility_Token toVisToken() {
			Visibility_Token token;
			token.insertType("Light_Directional");
			token.insertType("Light_Point");
			token.insertType("Light_Spot");

			if (m_list.size()) {
				// Find the closest element
				LightElement closest = m_list[0];
				unsigned int closestSpot = 0;
				for (unsigned int x = 1, size = m_list.size(); x < size; ++x) {
					const auto & element = m_list[x];
					if (element.m_importance > closest.m_importance) {
						closest = element;
						closestSpot = x;
					}
				}
				// Push the closest light first
				token[closest.m_lightType].push_back((Component*)closest.m_ptr);
				// Get the m_capacity-1 oldest lights
				for (unsigned int x = 0, size = m_list.size(); x < size && x < m_capacity-1; ++x) {
					if (x == closestSpot) 
						continue;
					const auto & element = m_list[x];
					token[element.m_lightType].push_back((Component*)element.m_ptr);
				}
			}
			return token;
		}

		// Members
		int m_capacity;
		vec3 m_position;
		vector<LightElement> m_list;
	};

	// Quit early if we don't have models, or we don't have any lights 
	// Logically equivalent to continuing while we have at least 1 model and 1 light
	if ((!vis_token.find("Anim_Model")) ||
		(!vis_token.find("Light_Directional"))  &&
		(!vis_token.find("Light_Point")) &&
		(!vis_token.find("Light_Spot")))
		return;

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_BACK);
		
	PriorityLightList timedList(m_updateQuality, m_enginePackage->m_Camera.getCameraBuffer().EyePosition);
	for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Directional"))
		timedList.add(component, "Light_Directional");
	for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Point"))
		timedList.add(component, "Light_Point");
	for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Spot"))
		timedList.add(component, "Light_Spot");


	const Visibility_Token &priorityLights = timedList.toVisToken();
	const auto &dirLights = priorityLights.getTypeList<Lighting_Component>("Light_Directional"),
			   &pointLights = priorityLights.getTypeList<Lighting_Component>("Light_Point"), 
			   &spotLights = priorityLights.getTypeList<Lighting_Component>("Light_Spot");
	m_shadowBuffer.bindForWriting(SHADOW_LARGE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_shaderDirectional_Shadow->bind();
	for each (auto &component in dirLights) 
		component->shadowPass();	

	m_shadowBuffer.bindForWriting(SHADOW_REGULAR);
	m_shaderPoint_Shadow->bind();
	for each (auto &component in pointLights) 
		component->shadowPass();	

	m_shaderSpot_Shadow->bind();
	for each (auto &component in spotLights) 
		component->shadowPass();	

	Asset_Shader::Release();	
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
}

void System_Graphics::geometryPass(const Visibility_Token & vis_token)
{
	if (vis_token.find("Anim_Model")) {
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glCullFace(GL_BACK);
		m_gBuffer.bindForWriting();
		m_shaderGeometry->bind();

		for each (auto &component in vis_token.getTypeList<Geometry_Component>("Anim_Model"))
			component->draw();

		Asset_Shader::Release();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_attribID);
		m_gBuffer.applyAO();
	}
}

void System_Graphics::skyPass()
{
	const size_t &quad_size = m_shapeQuad->getSize();
	m_lBuffer.bindForWriting();

	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_TEST);

	m_textureSky->bind(GL_TEXTURE0);
	m_shaderSky->bind();
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);
	glBindVertexArray(0);
	Asset_Shader::Release();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
}