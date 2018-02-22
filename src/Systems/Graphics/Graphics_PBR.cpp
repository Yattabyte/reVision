#include "Systems\Graphics\Graphics_PBR.h"
#include "Utilities\EnginePackage.h"
#include "Systems\World\Camera.h"
#include "Systems\World\ECS\Components\Geometry_Component.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include <random>
#include <minmax.h>


struct Primitivee_Observer : Asset_Observer
{
	Primitivee_Observer(Shared_Asset_Primitive & asset, const GLuint vao) : Asset_Observer(asset.get()), m_vao_id(vao) {};
	virtual void Notify_Finalized() {
		if (m_asset->existsYet())
			dynamic_pointer_cast<Asset_Primitive>(m_asset)->updateVAO(m_vao_id);
	}
	GLuint m_vao_id;
};
class SSAO_Callback : public Callback_Container {
public:
	~SSAO_Callback() {};
	SSAO_Callback(System_Graphics_PBR * graphics) : m_Graphics(graphics) {}
	void Callback(const float & value) {
		m_Graphics->setSSAO((bool)value);
	}
private:
	System_Graphics_PBR *m_Graphics;
};
class SSAO_Samples_Callback : public Callback_Container {
public:
	~SSAO_Samples_Callback() {};
	SSAO_Samples_Callback(System_Graphics_PBR * graphics) : m_Graphics(graphics) {}
	void Callback(const float & value) {
		m_Graphics->setSSAOSamples((int)value);
	}
private:
	System_Graphics_PBR *m_Graphics;
};
class SSAO_Strength_Callback : public Callback_Container {
public:
	~SSAO_Strength_Callback() {};
	SSAO_Strength_Callback(System_Graphics_PBR * graphics) : m_Graphics(graphics) {}
	void Callback(const float & value) {
		m_Graphics->setSSAOStrength((int)value);
	}
private:
	System_Graphics_PBR *m_Graphics;
};
class SSAO_Radius_Callback : public Callback_Container {
public:
	~SSAO_Radius_Callback() {};
	SSAO_Radius_Callback(System_Graphics_PBR * graphics) : m_Graphics(graphics) {}
	void Callback( const float & value) {
		m_Graphics->setSSAORadius(value);
	}
private:
	System_Graphics_PBR *m_Graphics;
};
class Bloom_StrengthChangeCallback : public Callback_Container {
public:
	~Bloom_StrengthChangeCallback() {};
	Bloom_StrengthChangeCallback(Lighting_Buffer * lBuffer) : m_LBuffer(lBuffer) {}
	void Callback(const float & value) {
		m_LBuffer->setBloomStrength((int)value);
	}
private:
	Lighting_Buffer *m_LBuffer;
};
class Cam_WidthChangeCallback : public Callback_Container {
public:
	~Cam_WidthChangeCallback() {};
	Cam_WidthChangeCallback(System_Graphics_PBR * graphics) : m_Graphics(graphics) {}
	void Callback(const float & value) {
		m_Graphics->resize(vec2(value, m_preferenceState->getPreference(PreferenceState::C_WINDOW_HEIGHT)));
	}
private:
	System_Graphics_PBR *m_Graphics;
};
class Cam_HeightChangeCallback : public Callback_Container {
public:
	~Cam_HeightChangeCallback() {};
	Cam_HeightChangeCallback(System_Graphics_PBR * lBuffer) : m_Graphics(lBuffer) {}
	void Callback(const float & value) {
		m_Graphics->resize(vec2(m_preferenceState->getPreference(PreferenceState::C_WINDOW_WIDTH), value));
	}
private:
	System_Graphics_PBR *m_Graphics;
}; 
class ShadowQualityChangeCallback : public Callback_Container {
public:
	~ShadowQualityChangeCallback() {};
	ShadowQualityChangeCallback(int * pointer) : m_pointer(pointer) {}
	void Callback(const float & value) {
		*m_pointer = (int)value;
	}
private:
	int *m_pointer;
};

System_Graphics_PBR::~System_Graphics_PBR()
{
	if (!m_Initialized) {
		m_enginePackage->removeCallback(PreferenceState::C_SSAO, m_ssaoCallback);
		m_enginePackage->removeCallback(PreferenceState::C_SSAO_SAMPLES, m_ssaoSamplesCallback);
		m_enginePackage->removeCallback(PreferenceState::C_SSAO_BLUR_STRENGTH, m_ssaoStrengthCallback);
		m_enginePackage->removeCallback(PreferenceState::C_SSAO_RADIUS, m_ssaoRadiusCallback);
		m_enginePackage->removeCallback(PreferenceState::C_WINDOW_HEIGHT, m_bloomStrengthChangeCallback);
		m_enginePackage->removeCallback(PreferenceState::C_WINDOW_WIDTH, m_widthChangeCallback);
		m_enginePackage->removeCallback(PreferenceState::C_WINDOW_HEIGHT, m_heightChangeCallback);
		m_enginePackage->removeCallback(PreferenceState::C_SHADOW_QUALITY, m_QualityChangeCallback);
		delete m_QuadObserver;
		delete m_ConeObserver;
		delete m_SphereObserver;
		delete m_ssaoCallback;
		delete m_ssaoSamplesCallback;
		delete m_ssaoStrengthCallback;
		delete m_ssaoRadiusCallback;
		delete m_bloomStrengthChangeCallback;
		delete m_widthChangeCallback;
		delete m_heightChangeCallback;
		delete m_QualityChangeCallback;
	}
}

System_Graphics_PBR::System_Graphics_PBR() :
	m_visualFX(), m_gBuffer(), m_lBuffer(), m_hdrBuffer(), m_giBuffer()
{
	m_quadVAO = 0;
	m_coneVAO = 0;
	m_sphereVAO = 0;
	m_bounceVAO = 0;
	m_attribID = 0;
	m_renderSize = vec2(1);
}

void System_Graphics_PBR::initialize(EnginePackage * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		Asset_Loader::load_asset(m_shaderGeometry, "Geometry\\geometry");
		Asset_Loader::load_asset(m_shaderDirectional_Shadow, "Geometry\\geometryShadowDir");
		Asset_Loader::load_asset(m_shaderPoint_Shadow, "Geometry\\geometryShadowPoint");
		Asset_Loader::load_asset(m_shaderSpot_Shadow, "Geometry\\geometryShadowSpot");
		Asset_Loader::load_asset(m_shaderDirectional, "Lighting\\directional");
		Asset_Loader::load_asset(m_shaderPoint, "Lighting\\point");
		Asset_Loader::load_asset(m_shaderSpot, "Lighting\\spot");
		Asset_Loader::load_asset(m_shaderDirectional_Bounce, "Lighting\\directional_bounce");
		Asset_Loader::load_asset(m_shaderPoint_Bounce, "Lighting\\point_bounce");
		Asset_Loader::load_asset(m_shaderSpot_Bounce, "Lighting\\spot_bounce");
		Asset_Loader::load_asset(m_shaderGIReconstruct, "Lighting\\gi_reconstruction");
		Asset_Loader::load_asset(m_shaderGISecondBounce, "Lighting\\gi_second_bounce");
		Asset_Loader::load_asset(m_shaderHDR, "FX\\HDR");
		Asset_Loader::load_asset(m_shaderFXAA, "FX\\FXAA");
		Asset_Loader::load_asset(m_shapeQuad, "quad");
		Asset_Loader::load_asset(m_shapeCone, "cone");
		Asset_Loader::load_asset(m_shapeSphere, "sphere");
		Asset_Loader::load_asset(m_shaderSky, "skybox");
		Asset_Loader::load_asset(m_textureSky, "sky\\");
		m_quadVAO = Asset_Primitive::Generate_VAO();
		m_coneVAO = Asset_Primitive::Generate_VAO();
		m_sphereVAO = Asset_Primitive::Generate_VAO();
		m_QuadObserver = (void*)(new Primitivee_Observer(m_shapeQuad, m_quadVAO));
		m_ConeObserver = (void*)(new Primitivee_Observer(m_shapeCone, m_coneVAO));
		m_SphereObserver = (void*)(new Primitivee_Observer(m_shapeSphere, m_sphereVAO));
		m_gBuffer.end();

		m_ssaoCallback = new SSAO_Callback(this);
		m_ssaoSamplesCallback = new SSAO_Samples_Callback(this);
		m_ssaoStrengthCallback = new SSAO_Strength_Callback(this);
		m_ssaoRadiusCallback = new SSAO_Radius_Callback(this);
		m_widthChangeCallback = new Cam_WidthChangeCallback(this);
		m_heightChangeCallback = new Cam_HeightChangeCallback(this);
		m_bloomStrengthChangeCallback = new Bloom_StrengthChangeCallback(&m_lBuffer);
		m_QualityChangeCallback = new ShadowQualityChangeCallback(&m_updateQuality);
		m_enginePackage->addCallback(PreferenceState::C_SSAO, m_ssaoCallback);
		m_enginePackage->addCallback(PreferenceState::C_SSAO_SAMPLES, m_ssaoSamplesCallback);
		m_enginePackage->addCallback(PreferenceState::C_SSAO_BLUR_STRENGTH, m_ssaoStrengthCallback);
		m_enginePackage->addCallback(PreferenceState::C_SSAO_RADIUS, m_ssaoRadiusCallback);
		m_enginePackage->addCallback(PreferenceState::C_WINDOW_WIDTH, m_widthChangeCallback);
		m_enginePackage->addCallback(PreferenceState::C_WINDOW_HEIGHT, m_heightChangeCallback);
		m_enginePackage->addCallback(PreferenceState::C_BLOOM_STRENGTH, m_bloomStrengthChangeCallback);
		m_enginePackage->addCallback(PreferenceState::C_SHADOW_QUALITY, m_QualityChangeCallback);
		m_attribs.m_ssao_radius = m_enginePackage->getPreference(PreferenceState::C_SSAO_RADIUS);
		m_attribs.m_ssao_strength = m_enginePackage->getPreference(PreferenceState::C_SSAO_BLUR_STRENGTH);
		m_attribs.m_aa_samples = m_enginePackage->getPreference(PreferenceState::C_SSAO_SAMPLES);
		m_attribs.m_ssao = m_enginePackage->getPreference(PreferenceState::C_SSAO);
		m_renderSize = vec2(m_enginePackage->getPreference(PreferenceState::C_WINDOW_WIDTH), m_enginePackage->getPreference(PreferenceState::C_WINDOW_HEIGHT));
		m_updateQuality = m_enginePackage->getPreference(PreferenceState::C_SHADOW_QUALITY);
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
		m_hdrBuffer.initialize(m_renderSize);
		m_lBuffer.initialize(m_renderSize, &m_visualFX, m_enginePackage->getPreference(PreferenceState::C_BLOOM_STRENGTH), m_gBuffer.m_depth_stencil);
		m_shadowBuffer.initialize(enginePackage);
		m_giBuffer.initialize(16, enginePackage);

		{
			GLuint VBO = 0;
			glGenVertexArrays(1, &m_bounceVAO);
			glBindVertexArray(m_bounceVAO);
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLint), GLint(0), GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribIPointer(0, 1, GL_INT, sizeof(GLint), 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

		m_Initialized = true;
	}
}

void System_Graphics_PBR::update(const float & deltaTime)
{
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);

	shared_lock<shared_mutex> read_guard(m_enginePackage->m_Camera.getDataMutex());
	Visibility_Token &vis_token = m_enginePackage->m_Camera.GetVisibilityToken();

	if (vis_token.size() &&
		m_shapeQuad->existsYet() &&
		m_textureSky->existsYet() &&
		m_shaderSky->existsYet() &&
		m_shaderGeometry->existsYet() &&
		m_shaderDirectional->existsYet())
	{
		// Regeneration Phase
		shadowPass(vis_token);
		bouncePass(vis_token);


		geometryPass(vis_token);
		skyPass();
		directLightingPass(vis_token);
		indirectLightingPass(vis_token);
		HDRPass();
		finalPass();
	}
}

void System_Graphics_PBR::setSSAO(const bool & ssao)
{
	m_attribs.m_ssao = ssao;
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics_PBR::setSSAOSamples(const int & samples)
{
	m_attribs.m_aa_samples = samples;
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics_PBR::setSSAOStrength(const int & strength)
{
	m_attribs.m_ssao_strength = strength;
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics_PBR::setSSAORadius(const float & radius)
{
	m_attribs.m_ssao_radius = radius;
	glBindBuffer(GL_UNIFORM_BUFFER, m_attribID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Renderer_Attribs), &m_attribs);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void System_Graphics_PBR::resize(const vec2 & size)
{
	m_renderSize = size;
	m_gBuffer.resize(size);
	m_lBuffer.resize(size);
	m_hdrBuffer.resize(size);
}

Shadow_Buffer & System_Graphics_PBR::getShadowBuffer()
{
	return m_shadowBuffer;
}

void System_Graphics_PBR::generateKernal()
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

void System_Graphics_PBR::shadowPass(const Visibility_Token & vis_token)
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

void System_Graphics_PBR::bouncePass(const Visibility_Token & vis_token)
{
	m_giBuffer.updateData();

	// Prepare rendering state
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	m_giBuffer.bindNoise(GL_TEXTURE4);
	m_giBuffer.bindForWriting(0);

	// Perform primary light bounce
	glBindVertexArray(m_bounceVAO);

		// Bounce directional light
		m_shadowBuffer.BindForReading_GI(SHADOW_LARGE, GL_TEXTURE0);
		if (vis_token.find("Light_Directional")) {
			m_shaderDirectional_Bounce->bind();
			for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Directional"))
				component->indirectPass(16);
		}
		// Bounce point lights
		m_shadowBuffer.BindForReading_GI(SHADOW_REGULAR, GL_TEXTURE0);
		if (vis_token.find("Light_Point")) {
			m_shaderPoint_Bounce->bind();
			for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Point"))
				component->indirectPass(16);
		}
		// Bounce spot lights
		if (vis_token.find("Light_Spot")) {
			m_shaderSpot_Bounce->bind();
			for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Spot"))
				component->indirectPass(16);
		}		

	// Perform secondary light bounce
	m_shaderGISecondBounce->bind();
	m_giBuffer.bindForReading(0, GL_TEXTURE5);
	m_giBuffer.bindForWriting(1);
	glDrawArraysInstanced(GL_POINTS, 0, 1, 32);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	Asset_Shader::Release();
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
}

void System_Graphics_PBR::geometryPass(const Visibility_Token & vis_token)
{
	if (vis_token.find("Anim_Model")) {
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glCullFace(GL_BACK);
		m_gBuffer.clear();
		m_gBuffer.bindForWriting();
		m_shaderGeometry->bind();

		for each (auto &component in vis_token.getTypeList<Geometry_Component>("Anim_Model"))
			component->draw();

		Asset_Shader::Release();
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_attribID);
		m_gBuffer.applyAO();
	}
}

void System_Graphics_PBR::skyPass()
{
	const size_t &quad_size = m_shapeQuad->getSize();
	m_lBuffer.clear();
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

void System_Graphics_PBR::directLightingPass(const Visibility_Token & vis_token)
{
	const size_t &quad_size = m_shapeQuad->getSize();
	const size_t &cone_size = m_shapeCone->getSize();
	const size_t &sphere_size = m_shapeSphere->getSize();
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	m_gBuffer.bindForReading();
	m_lBuffer.bindForWriting();

	m_shadowBuffer.bindForReading(SHADOW_LARGE, 3);
	if (vis_token.find("Light_Directional")) {
		m_shaderDirectional->bind();
		glBindVertexArray(m_quadVAO);
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Directional"))
			component->directPass(quad_size);
	}

	glEnable(GL_STENCIL_TEST);
	glCullFace(GL_FRONT);
	m_shadowBuffer.bindForReading(SHADOW_REGULAR, 3);
	if (vis_token.find("Light_Point")) {
		m_shaderPoint->bind();
		glBindVertexArray(m_sphereVAO);
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Point"))
			component->directPass(sphere_size);
	}

	if (vis_token.find("Light_Spot")) {
		m_shaderSpot->bind();
		glBindVertexArray(m_coneVAO);
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Spot"))
			component->directPass(cone_size);
	}

	glCullFace(GL_BACK);
	glDisable(GL_STENCIL_TEST);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);
	Asset_Shader::Release();
	glBindVertexArray(0);
	m_lBuffer.applyBloom();
}

void System_Graphics_PBR::indirectLightingPass(const Visibility_Token & vis_token)
{
	// Reconstruct GI from data
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	m_gBuffer.bindForReading();
	m_lBuffer.bindForWriting();

	m_shaderGIReconstruct->bind();
	const size_t &quad_size = m_shapeQuad->getSize();	

	m_giBuffer.bindNoise(GL_TEXTURE4);
	glBindVertexArray(m_quadVAO);
	m_giBuffer.bindForReading(1, GL_TEXTURE5);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);
	

	Asset_Shader::Release();
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void System_Graphics_PBR::HDRPass()
{
	const size_t &quad_size = m_shapeQuad->getSize();
	m_lBuffer.bindForReading();
	m_hdrBuffer.clear();
	m_hdrBuffer.bindForWriting();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
	//glDisable(GL_CULL_FACE);

	m_shaderHDR->bind();
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);
	glBindVertexArray(0);
	Asset_Shader::Release();
}

void System_Graphics_PBR::finalPass()
{
	const size_t &quad_size = m_shapeQuad->getSize();
	m_hdrBuffer.bindForReading();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	m_shaderFXAA->bind();
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);
	glBindVertexArray(0);
	Asset_Shader::Release();
	m_gBuffer.end();	
}