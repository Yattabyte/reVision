#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\DS_Lighting.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\GlobalIllumination_RH.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflections_SSR.h"
#include "Systems\Graphics\FX Techniques\Bloom_Tech.h"
#include "Systems\Graphics\FX Techniques\HDR_Tech.h"
#include "Systems\Graphics\FX Techniques\FXAA_Tech.h"
#include "Utilities\EnginePackage.h"
#include "Systems\World\Camera.h"
#include "Systems\World\ECS\Components\Geometry_Component.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Systems\World\ECS\Components\Reflector_Component.h"
#include "Utilities\PriorityList.h"
#include <random>
#include <minmax.h>


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
		if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);

		for each (auto * tech in m_lightingTechs)
			delete tech;
		for each (auto * tech in m_fxTechs)
			delete tech;
	}
}

System_Graphics::System_Graphics() :
	m_visualFX(), m_geometryFBO(), m_lightingFBO(), m_shadowBuffer(), m_reflectionFBO()
{
	m_quadVAO = 0;
	m_renderSize = vec2(1);
}

void System_Graphics::initialize(EnginePackage * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;

		// Load Assets
		Asset_Loader::load_asset(m_shaderGeometry, "Geometry\\geometry");
		Asset_Loader::load_asset(m_shaderDirectional_Shadow, "Geometry\\geometryShadowDir");
		Asset_Loader::load_asset(m_shaderPoint_Shadow, "Geometry\\geometryShadowPoint");
		Asset_Loader::load_asset(m_shaderSpot_Shadow, "Geometry\\geometryShadowSpot");
		Asset_Loader::load_asset(m_shapeQuad, "quad");
		Asset_Loader::load_asset(m_shaderSky, "skybox");
		Asset_Loader::load_asset(m_textureSky, "sky\\");

		m_quadVAO = Asset_Primitive::Generate_VAO();
		m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); });
		m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(ivec2(f, m_renderSize.y)); });
		m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(ivec2(m_renderSize.x, f)); });
		m_updateQuality = m_enginePackage->addPrefCallback(PreferenceState::C_SHADOW_QUALITY, this, [&](const float &f) {setShadowUpdateQuality(f); });

		// Generate User SSBO
		struct Renderer_Attribs {
			vec4 kernel[MAX_KERNEL_SIZE];
			float m_ssao_radius;
			int m_ssao_strength, m_aa_samples;
			int m_ssao;
		};
		Renderer_Attribs attribs;
		attribs.m_ssao = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO, this, [&](const float &f) {setSSAO(f); });
		attribs.m_aa_samples = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO_SAMPLES, this, [&](const float &f) {setSSAOSamples(f); });
		attribs.m_ssao_strength = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO_BLUR_STRENGTH, this, [&](const float &f) {setSSAOStrength(f); });
		attribs.m_ssao_radius = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO_RADIUS, this, [&](const float &f) {setSSAORadius(f); });		
		m_userBuffer = MappedBuffer(sizeof(Renderer_Attribs), &attribs);
		m_userBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
		generateKernal();

		// Generate Visibility SSBO
		m_vishadowFBO = MappedBuffer(sizeof(GLuint) * 500, 0);
		m_vishadowFBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);

		// Initiate graphics buffers
		m_visualFX.initialize(enginePackage);
		m_geometryFBO.initialize(enginePackage, &m_visualFX);
		m_lightingFBO.initialize(enginePackage, m_geometryFBO.m_depth_stencil);
		m_shadowBuffer.initialize(enginePackage);
		m_reflectionFBO.initialize(enginePackage);

		// Initiate lighting techniques
		m_lightingTechs.push_back(new DS_Lighting(&m_geometryFBO, &m_lightingFBO, &m_shadowBuffer));
		m_lightingTechs.push_back(new GlobalIllumination_RH(m_enginePackage, &m_geometryFBO, &m_lightingFBO, &m_shadowBuffer)); 
		m_lightingTechs.push_back(new Reflections_SSR(m_enginePackage, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO, &m_visualFX));
		m_fxTechs.push_back(new Bloom_Tech(enginePackage, &m_lightingFBO, &m_visualFX));

		// Initiate effects techniques
		m_fxTechs.push_back(new HDR_Tech(enginePackage));
		m_fxTechs.push_back(new FXAA_Tech());
		m_Initialized = true;


		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}
}

void System_Graphics::update(const float & deltaTime)
{
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);

	const Visibility_Token vis_token = m_enginePackage->m_Camera.getVisibilityToken();

	if (m_Initialized &&
		vis_token.size() &&
		m_shapeQuad->existsYet() &&
		m_textureSky->existsYet() &&
		m_shaderSky->existsYet() &&
		m_shaderGeometry->existsYet())
	{
		// Update buffers and bind initial state
		updateBuffers(vis_token);

		// Regeneration Phase
		shadowPass(vis_token);
		for each (auto *tech in m_lightingTechs)
			tech->updateLighting(vis_token);

		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
		m_geometryFBO.clear();
		m_lightingFBO.clear();
		m_reflectionFBO.clear();
		geometryPass(vis_token);
		
		// Writing to lighting fbo
		skyPass();		
		//m_lightingTechs[1]->applyLighting(vis_token);/*
		for each (auto *tech in m_lightingTechs)
			tech->applyLighting(vis_token);

		// Chain of post-processing effects ending in default fbo
		m_lightingFBO.bindForReading();
		for each (auto *tech in m_fxTechs) {
			tech->applyEffect();
			tech->bindForReading();
		}

		m_lightingFBO.end();
		m_geometryFBO.end();
	}
}

void System_Graphics::setSSAO(const bool & ssao)
{
	//float m_ssao_radius;
	//int m_ssao_strength, m_aa_samples;
	//int m_ssao;
	m_userBuffer.write((sizeof(vec4) * MAX_KERNEL_SIZE) + (sizeof(int) * 3), sizeof(int), &ssao);
}

void System_Graphics::setSSAOSamples(const int & samples)
{
	m_userBuffer.write((sizeof(vec4) * MAX_KERNEL_SIZE) + (sizeof(int) * 2), sizeof(int), &samples);
}

void System_Graphics::setSSAOStrength(const int & strength)
{
	m_userBuffer.write((sizeof(vec4) * MAX_KERNEL_SIZE) + (sizeof(int)), sizeof(int), &strength);
}

void System_Graphics::setSSAORadius(const float & radius)
{
	m_userBuffer.write((sizeof(vec4) * MAX_KERNEL_SIZE), sizeof(float), &radius);
}

void System_Graphics::resize(const ivec2 & size)
{
	m_renderSize = size;
}

void System_Graphics::setShadowUpdateQuality(const float & quality)
{
	m_updateQuality = quality;
}

void System_Graphics::generateKernal()
{
	vec4 kernel[MAX_KERNEL_SIZE];
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
		kernel[t] = vec4(sample, 1);
	}

	// Write to buffer, kernel is first part of this buffer
	m_userBuffer.write(0, sizeof(vec4)*MAX_KERNEL_SIZE, kernel);
}

void System_Graphics::updateBuffers(const Visibility_Token & vis_token)
{
	// Update reflectors
	m_vishadowFBO.checkFence();
	vector<GLuint> visArray(vis_token.specificSize("Reflector"));
	unsigned int count = 0;
	for each (const auto &component in vis_token.getTypeList<Reflector_Component>("Reflector")) 
		visArray[count++] = component->getBufferIndex();	
	m_vishadowFBO.write(0, sizeof(GLuint)*visArray.size(), visArray.data());

	m_vishadowFBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	m_userBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
}

void System_Graphics::shadowPass(const Visibility_Token & vis_token)
{
	/** This is used to prioritize the oldest AND closest lights to the viewer (position) **/
	class PriorityLightList
	{
	public:
		// (de)Constructors
		/** Default destructor. */
		~PriorityLightList() {}
		/** Construct a priority light list with the given quality and position.
		 * @param	quality		the max number of final elements
		 * @param	position	the position of the viewer */
		PriorityLightList(const unsigned int & quality, const vec3 & position) : m_quality(quality), m_oldest(quality), m_position(position) {}


		// Public Methods
		/** Fill the oldest light list with a new light, and have it sorted.
		 * @param	light		the light to insert */
		void insert(Lighting_Component * light) {
			m_oldest.insert(light->getShadowUpdateTime(), light);
		}
		/** Return a list composed of the oldest and the closest lights.
		 * @return				a double sorted list with the oldest lights and closest lights */
		const vector<Lighting_Component*> toList() const {
			PriorityList<float, Lighting_Component*, greater<float>> m_closest(m_quality / 2);
			vector<Lighting_Component*> outList;
			outList.reserve(m_quality);

			for each (const auto &element in m_oldest.toList()) {
				if (outList.size() < (m_quality / 2))
					outList.push_back(element);
				else
					m_closest.insert(element->getImportance(m_position), element);
			}

			for each (const auto &element in m_closest.toList()) {
				if (outList.size() > m_quality)
					break;
				outList.push_back(element);
			}
			return outList;
		}


	private:
		// Private Attributes
		unsigned int m_quality;
		vec3 m_position;
		PriorityList<float, Lighting_Component*, less<float>> m_oldest;
	};
	
	// Quit early if we don't have models, or we don't have any lights 
	// Logically equivalent to continuing while we have at least 1 model and 1 light
	if ((!vis_token.find("Anim_Model")) ||
		(!vis_token.find("Light_Directional"))  &&
		(!vis_token.find("Light_Point")) &&
		(!vis_token.find("Light_Spot")))
		return;

	// Retrieve a sorted list of most important lights to run shadow calc for.
	const vec3 &camPos = m_enginePackage->m_Camera.getCameraBuffer().EyePosition;
	PriorityLightList queueDir(m_updateQuality, camPos), queuePoint(m_updateQuality, camPos), queueSpot(m_updateQuality, camPos);
	for each (const auto &component in vis_token.getTypeList<Lighting_Component>("Light_Directional"))
		queueDir.insert(component);
	for each (const auto &component in vis_token.getTypeList<Lighting_Component>("Light_Point"))
		queuePoint.insert(component);
	for each (const auto &component in vis_token.getTypeList<Lighting_Component>("Light_Spot"))
		queueSpot.insert(component);
	const auto &dirLights = queueDir.toList();
	const auto &pointLights = queuePoint.toList();
	const auto &spotLights = queueSpot.toList();

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glCullFace(GL_BACK);	
	
	m_shadowBuffer.bindForWriting(SHADOW_LARGE);
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
		glEnable(GL_CULL_FACE);
		m_geometryFBO.bindForWriting();
		m_shaderGeometry->bind();

		for each (auto &component in vis_token.getTypeList<Geometry_Component>("Anim_Model"))
			component->draw();

		Asset_Shader::Release();
		m_geometryFBO.applyAO();
	}
}

void System_Graphics::skyPass()
{
	const size_t &quad_size = m_shapeQuad->getSize();
	m_lightingFBO.bindForWriting();

	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_TEST);

	m_textureSky->bind(0);
	m_shaderSky->bind();
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);
	glBindVertexArray(0);
	Asset_Shader::Release();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
}