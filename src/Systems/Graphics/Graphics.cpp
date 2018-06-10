#include "Systems\Graphics\Graphics.h"
#include "Systems\World\Camera.h"
#include "Managers\Material_Manager.h"
#include "Utilities\EnginePackage.h"
#include <random>
#include <minmax.h>

// Begin incldues for specific techniques
#include "Systems\Graphics\Resources\Geometry Techniques\Model_Static_Technique.h"
#include "Systems\Graphics\Resources\Geometry Techniques\Model_Technique.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Sky Lighting\Skybox.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\DS_Lighting.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\GlobalIllumination_RH.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflections.h"
#include "Systems\Graphics\FX Techniques\Bloom_Tech.h"
#include "Systems\Graphics\FX Techniques\HDR_Tech.h"
#include "Systems\Graphics\FX Techniques\FXAA_Tech.h"
// End includes for specific techniques

// Begin includes for specific lighting techniques
#include "Systems\Graphics\Resources\Lighting Techniques\Base Types\Directional.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Base Types\Directional_Cheap.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Base Types\Spot.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Base Types\Spot_Cheap.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Base Types\Point.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Base Types\Point_Cheap.h"
// End includes for specific lighting techniques


System_Graphics::~System_Graphics()
{
	if (!m_Initialized) {
		m_enginePackage->removePrefCallback(PreferenceState::C_SSAO, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_SSAO_QUALITY, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_SSAO_BLUR_STRENGTH, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_SSAO_RADIUS, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);

		for each (auto * tech in m_lightingTechs)
			delete tech;
		for each (auto * tech in m_fxTechs)
			delete tech;
	}
}

System_Graphics::System_Graphics() : m_visualFX(), m_geometryFBO(), m_lightingFBO(), m_reflectionFBO(), m_renderSize(vec2(1.0f)) {}

void System_Graphics::initialize(EnginePackage * enginePackage)
{
	if (!m_Initialized) {
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		m_enginePackage = enginePackage;

		// Generate User SSBO		
		Renderer_Struct attribs;
		attribs.m_ssao = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO, this, [&](const float &f) {
			m_userBuffer.write(offsetof(Renderer_Struct, m_ssao), sizeof(int), &f);
		});
		attribs.m_aa_quality = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO_QUALITY, this, [&](const float &f) {
			m_userBuffer.write(offsetof(Renderer_Struct, m_aa_quality), sizeof(int), &f);
		});
		attribs.m_ssao_strength = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO_BLUR_STRENGTH, this, [&](const float &f) {
			m_userBuffer.write(offsetof(Renderer_Struct, m_ssao_strength), sizeof(int), &f);
		});
		attribs.m_ssao_radius = m_enginePackage->addPrefCallback(PreferenceState::C_SSAO_RADIUS, this, [&](const float &f) {
			m_userBuffer.write(offsetof(Renderer_Struct, m_ssao_radius), sizeof(float), &f);
		});		
		m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {
			m_renderSize = ivec2(f, m_renderSize.y); 
		});
		m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {
			m_renderSize = ivec2(m_renderSize.x, f); 
		});
		m_userBuffer = StaticBuffer(sizeof(Renderer_Struct), &attribs);
		m_userBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
		generateKernal();

		// Initiate graphics buffers
		m_visualFX.initialize(enginePackage);
		m_geometryFBO.initialize(enginePackage, &m_visualFX);
		m_lightingFBO.initialize(enginePackage, m_geometryFBO.m_depth_stencil);
		m_reflectionFBO.initialize(enginePackage, m_geometryFBO.m_depth_stencil);

		// Initiate base lighting techniques
		m_baseTechs.push_back(new Directional_Tech(enginePackage, &m_lightBuffers));
		m_baseTechs.push_back(new Spot_Tech(enginePackage, &m_lightBuffers));
		m_baseTechs.push_back(new Point_Tech(enginePackage, &m_lightBuffers));
		m_baseTechs.push_back(new Directional_Tech_Cheap(&m_lightBuffers));
		m_baseTechs.push_back(new Spot_Cheap_Tech(&m_lightBuffers));
		m_baseTechs.push_back(new Point_Tech_Cheap(&m_lightBuffers));
		for each(auto * tech in m_baseTechs)
			m_techMap[tech->getName()] = tech;

		// Initiate specialized geometry techniques
		m_geometryTechs.push_back(new Model_Static_Technique(&m_geometryFBO, &m_geometryBuffers.m_geometryStaticSSBO));
		m_geometryTechs.push_back(new Model_Technique(&m_geometryFBO, &m_geometryBuffers.m_geometryDynamicSSBO));

		// Initiate specialized lighting techniques
		m_lightingTechs.push_back(new Skybox(&m_lightingFBO));
		m_lightingTechs.push_back(new DS_Lighting(m_enginePackage, &m_geometryFBO, &m_lightingFBO, &m_baseTechs, &m_geometryBuffers));
		m_lightingTechs.push_back(new GlobalIllumination_RH(m_enginePackage, &m_geometryFBO, &m_lightingFBO, &m_baseTechs));
		m_lightingTechs.push_back(new Reflections(m_enginePackage, &m_geometryFBO, &m_lightingFBO, &m_reflectionFBO));

		// Initiate specialized effects techniques
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
		Material_Manager::Bind();
		m_userBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);

		send2GPU(vis_token);
		updateOnGPU(vis_token);
		renderFrame(vis_token);

		m_reflectionFBO.end();
		m_lightingFBO.end();
		m_geometryFBO.end();
		Asset_Shader::Release();
	}
}

void System_Graphics::generateKernal()
{
	vec4 new_kernel[MAX_KERNEL_SIZE];
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
		new_kernel[t] = vec4(sample, 1);
	}

	// Write to buffer, kernel is first part of this buffer
	m_userBuffer.write(offsetof(Renderer_Struct, kernel[0]), sizeof(vec4)*MAX_KERNEL_SIZE, new_kernel);
}

void System_Graphics::send2GPU(const Visibility_Token & vis_token)
{
	// Geometry Data
	Model_Static_Technique::writeCameraBuffers(m_enginePackage->m_Camera);
	Model_Technique::writeCameraBuffers(m_enginePackage->m_Camera);
	// Lighting Data
	for each (auto *tech in m_lightingTechs)
		tech->updateData(vis_token);
}

void System_Graphics::updateOnGPU(const Visibility_Token & vis_token)
{
	// Update Render Lists
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	for each (auto *tech in m_geometryTechs)
		tech->occlusionCullBuffers(m_enginePackage->m_Camera);
	// Shadows & Global Illumination
	for each (auto *tech in m_lightingTechs)
		tech->applyPrePass(vis_token);
}

void System_Graphics::renderFrame(const Visibility_Token & vis_token)
{
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	m_geometryFBO.clear();
	m_lightingFBO.clear();
	m_reflectionFBO.clear();

	// Geometry
	for each (auto *tech in m_geometryTechs)
		tech->renderGeometry(m_enginePackage->m_Camera);
	m_geometryFBO.applyAO();

	// Lighting
	m_reflectionSSBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 5); // PASS INTO RESPECTIVE TECHNIQUES
	for each (auto *tech in m_lightingTechs)
		tech->applyLighting(vis_token);

	// Post Processing
	m_lightingFBO.bindForReading();
	for each (auto *tech in m_fxTechs) {
		tech->applyEffect();
		tech->bindForReading();
	}
}