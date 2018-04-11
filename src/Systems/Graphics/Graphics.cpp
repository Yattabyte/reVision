#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Resources\Geometry Techniques\Model_Techniques.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Sky Lighting\Skybox.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\DS_Lighting.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\GlobalIllumination_RH.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflections.h"
#include "Systems\Graphics\FX Techniques\Bloom_Tech.h"
#include "Systems\Graphics\FX Techniques\HDR_Tech.h"
#include "Systems\Graphics\FX Techniques\FXAA_Tech.h"
#include "Systems\World\Camera.h"
#include "Managers\Material_Manager.h"
#include "Utilities\EnginePackage.h"
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

		for each (auto * tech in m_lightingTechs)
			delete tech;
		for each (auto * tech in m_fxTechs)
			delete tech;
	}
}

System_Graphics::System_Graphics() : m_visualFX(), m_geometryFBO(), m_lightingFBO(), m_shadowFBO(), m_reflectionFBO(), m_renderSize(vec2(1.0f)) {}

void System_Graphics::initialize(EnginePackage * enginePackage)
{
	if (!m_Initialized) {
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		m_enginePackage = enginePackage;

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
		m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {m_renderSize = ivec2(f, m_renderSize.y); });
		m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {m_renderSize = ivec2(m_renderSize.x, f); });
		m_userBuffer = StaticBuffer(sizeof(Renderer_Attribs), &attribs);
		m_userBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
		generateKernal();

		// Initiate graphics buffers
		m_visualFX.initialize(enginePackage);
		m_geometryFBO.initialize(enginePackage, &m_visualFX);
		m_lightingFBO.initialize(enginePackage, m_geometryFBO.m_depth_stencil);
		m_reflectionFBO.initialize(enginePackage, m_geometryFBO.m_depth_stencil);
		m_shadowFBO.initialize(enginePackage);

		// Initiate geometry techniques
		m_geometryTechs.push_back(new Model_Technique(enginePackage, &m_geometryFBO, &m_shadowFBO));

		// Initiate lighting techniques
		m_lightingTechs.push_back(new Skybox(&m_lightingFBO));
		m_lightingTechs.push_back(new DS_Lighting(&m_geometryFBO, &m_lightingFBO, &m_shadowFBO));
		m_lightingTechs.push_back(new GlobalIllumination_RH(m_enginePackage, &m_geometryFBO, &m_lightingFBO, &m_shadowFBO)); 
		m_lightingTechs.push_back(new Reflections(m_enginePackage, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO));

		// Initiate effects techniques
		m_fxTechs.push_back(new Bloom_Tech(enginePackage, &m_lightingFBO, &m_visualFX));
		m_fxTechs.push_back(new HDR_Tech(enginePackage));
		m_fxTechs.push_back(new FXAA_Tech());
		m_Initialized = true;
	}
}

void System_Graphics::update(const float & deltaTime)
{
	const Visibility_Token vis_token = m_enginePackage->m_Camera.getVisibilityToken();
	if (m_Initialized && vis_token.size())	{		
		// Update and bind prerequisite data
		Material_Manager::Bind();
		m_userBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
		m_geometrySSBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 5);
		for each (auto *tech in m_geometryTechs)
			tech->updateData(vis_token); 
		for each (auto *tech in m_lightingTechs)
			tech->updateData(vis_token);

		// Shadows
		m_lightDirSSBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6); // PASS INTO RESPECTIVE TECHNIQUES
		for each (auto *tech in m_geometryTechs)
			tech->renderShadows(vis_token);
		
		// Geometry
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
		m_geometryFBO.clear();
		m_lightingFBO.clear();
		m_reflectionFBO.clear();		
		for each (auto *tech in m_geometryTechs)
			tech->renderGeometry(vis_token);
		
		// Lighting
		m_reflectionSSBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 5); // PASS INTO RESPECTIVE TECHNIQUES
		m_lightDirSSBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6); // PASS INTO RESPECTIVE TECHNIQUES
		for each (auto *tech in m_lightingTechs)
			tech->applyLighting(vis_token);

		// Post Processing
		m_lightingFBO.bindForReading();
		for each (auto *tech in m_fxTechs) {
			tech->applyEffect();
			tech->bindForReading();
		}

		// End and reset
		m_reflectionFBO.end();
		m_lightingFBO.end();
		m_geometryFBO.end();
		Asset_Shader::Release();
	}
}

void System_Graphics::setSSAO(const bool & ssao)
{
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