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

		glDeleteBuffers(1, &m_attribID);
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

		// Load preferences + callbacks
		m_attribs.m_ssao = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO, this, [&](const float &f) {setSSAO(f); });
		m_attribs.m_aa_samples = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO_SAMPLES, this, [&](const float &f) {setSSAOSamples(f); });
		m_attribs.m_ssao_strength = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO_BLUR_STRENGTH, this, [&](const float &f) {setSSAOStrength(f); });
		m_attribs.m_ssao_radius = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO_RADIUS, this, [&](const float &f) {setSSAORadius(f); });
		m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(vec2(f, m_renderSize.y)); });
		m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(vec2(m_renderSize.x, f)); });
		m_updateQuality = m_enginePackage->addPrefCallback(PreferenceState::C_SHADOW_QUALITY, this, [&](const float &f) {setShadowUpdateQuality(f); });		

		// Generate attribute buffer
		glGenBuffers(1, &m_attribID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_attribID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_attribs), &m_attribs, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_attribID);
		GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
		memcpy(p, &m_attribs, sizeof(m_attribs));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		generateKernal();
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

		// Initiate graphics buffers
		m_visualFX.initialize(m_enginePackage);
		m_gBuffer.initialize(m_renderSize, &m_visualFX);
		m_lBuffer.initialize(m_renderSize, m_gBuffer.m_depth_stencil);
		m_shadowBuffer.initialize(enginePackage);

		// Initiate lighting techniques
		m_lightingTechs.push_back(new DirectLighting_Tech(&m_gBuffer, &m_lBuffer, &m_shadowBuffer));
		m_lightingTechs.push_back(new IndirectDiffuse_GI_Tech(m_enginePackage, &m_gBuffer, &m_lBuffer, &m_shadowBuffer)); 
		m_lightingTechs.push_back(new IndirectSpecular_SSR_Tech(m_enginePackage, &m_gBuffer, &m_lBuffer, &m_visualFX));
		m_fxTechs.push_back(new Bloom_Tech(enginePackage, &m_lBuffer, &m_visualFX));

		// Initiate effects techniques
		m_fxTechs.push_back(new HDR_Tech(enginePackage));
		m_fxTechs.push_back(new FXAA_Tech());
		m_Initialized = true;
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