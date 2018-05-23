#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\GlobalIllumination_RH.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Systems\World\World.h"
#include "Systems\Graphics\Graphics.h"
#include "Utilities\EnginePackage.h"
#include "Managers\Message_Manager.h"
#include <algorithm>
#include "GLM\gtc\matrix_transform.hpp"
#include <random>


GlobalIllumination_RH::~GlobalIllumination_RH()
{
	glDeleteTextures(1, &m_noise32);
	glDeleteTextures(GI_LIGHT_BOUNCE_COUNT * GI_TEXTURE_COUNT, m_textures[0]);
	glDeleteFramebuffers(GI_LIGHT_BOUNCE_COUNT, m_fbo);
	glDeleteVertexArrays(1, &m_bounceVAO);
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	m_enginePackage->getSubSystem<System_World>("World")->unregisterViewer(&m_camera);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
	m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
}

GlobalIllumination_RH::GlobalIllumination_RH(EnginePackage * enginePackage, Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, vector<Light_Tech*> * baseTechs)
{
	m_enginePackage = enginePackage;

	// FBO's
	m_geometryFBO = geometryFBO;
	m_lightingFBO = lightingFBO;

	// Base Techniques
	m_baseTechs = baseTechs;

	m_nearPlane = -0.1f;
	m_farPlane = -1.0f;
	m_noise32 = 0;
	ZERO_MEM(m_fbo);
	ZERO_MEM(m_textures[0]);
	ZERO_MEM(m_textures[1]);

	Asset_Loader::load_asset(m_shaderGISecondBounce, "Lighting\\Indirect Lighting\\Global Illumination (diffuse)\\gi_second_bounce");
	Asset_Loader::load_asset(m_shaderGIReconstruct, "Lighting\\Indirect Lighting\\Global Illumination (diffuse)\\gi_reconstruction");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	m_vaoLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); m_vaoLoaded = true; });
	m_enginePackage->getSubSystem<System_World>("World")->registerViewer(&m_camera);

	m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {ivec2(f, m_renderSize.y); });
	m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {ivec2(m_renderSize.x, f); });
	
	m_resolution = 16;
	GI_Radiance_Struct attribData(16, m_resolution, 0.001, 0, 25.0f);
	m_attribBuffer = StaticBuffer(sizeof(GI_Radiance_Struct), &attribData);

	GLuint secondBounceData[4] = { 6, m_resolution, 0, 0 }; // count, primCount, first, reserved
	m_IndirectSecondLayersBuffer = StaticBuffer(sizeof(GLuint) * 4, secondBounceData);

	GLuint quadData[4] = { 6, 1, 0, 0 }; // count, primCount, first, reserved
	m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData);

	// Pretend we have 4 cascades, and make the desired far plane only as far as the first would go
	const float near_plane = m_nearPlane;
	const float far_plane = -m_enginePackage->getPreference(PreferenceState::C_DRAW_DISTANCE);
	const float cLog = near_plane * powf((far_plane / near_plane), (1.0f / 4.0f));
	const float cUni = near_plane + ((far_plane - near_plane) * 1.0f / 4.0f);
	const float lambda = 0.4f;
	m_farPlane = (lambda*cLog) + ((1.0f - lambda)*cUni);

	GLuint VBO = 0;
	glCreateVertexArrays(1, &m_bounceVAO);
	glCreateBuffers(1, &VBO);
	glEnableVertexArrayAttrib(m_bounceVAO, 0);
	glNamedBufferStorage(VBO, sizeof(GLint), GLint(0), GL_CLIENT_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	glVertexArrayAttribIFormat(m_bounceVAO, 0, 1, GL_INT, 0);
	glVertexArrayVertexBuffer(m_bounceVAO, 0, VBO, 0, 4);
	glVertexArrayAttribBinding(m_bounceVAO, 0, 0);
	glDeleteBuffers(1, &VBO);

	glCreateFramebuffers(GI_LIGHT_BOUNCE_COUNT, m_fbo);
	glCreateTextures(GL_TEXTURE_3D, GI_LIGHT_BOUNCE_COUNT * GI_TEXTURE_COUNT, m_textures[0]);
	for (int bounce = 0; bounce < GI_LIGHT_BOUNCE_COUNT; ++bounce) {		
		for (int channel = 0; channel < GI_TEXTURE_COUNT; ++channel) {
			glTextureParameteri(m_textures[bounce][channel], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textures[bounce][channel], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textures[bounce][channel], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textures[bounce][channel], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_textures[bounce][channel], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureImage3DEXT(m_textures[bounce][channel], GL_TEXTURE_3D, 0, GL_RGBA16F, m_resolution, m_resolution, m_resolution, 0, GL_RGBA, GL_FLOAT, 0);
			glNamedFramebufferTexture(m_fbo[bounce], GL_COLOR_ATTACHMENT0 + channel, m_textures[bounce][channel], 0);
		}
		GLenum Buffers[] = {GL_COLOR_ATTACHMENT0,
							GL_COLOR_ATTACHMENT1,
							GL_COLOR_ATTACHMENT2,
							GL_COLOR_ATTACHMENT3};
		glNamedFramebufferDrawBuffers(m_fbo[bounce], 4, Buffers);
		GLenum Status = glCheckNamedFramebufferStatus(m_fbo[bounce], GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
			std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
			MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "Lighting Buffer", errorString);
			return;
		}
	}

	// Generate Noise Texture
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine generator;
	vec3 data[32 * 32 * 32];
	for (int x = 0, total = (32 * 32 * 32); x < total; ++x)
		data[x] = vec3(randomFloats(generator), randomFloats(generator), randomFloats(generator));
	glCreateTextures(GL_TEXTURE_3D, 1, &m_noise32);
	glTextureImage3DEXT(m_noise32, GL_TEXTURE_3D, 0, GL_RGB16F, 32, 32, 32, 0, GL_RGB, GL_FLOAT, &data);
	glTextureParameteri(m_noise32, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTextureParameteri(m_noise32, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTextureParameteri(m_noise32, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
	glTextureParameteri(m_noise32, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_noise32, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
}

void GlobalIllumination_RH::updateData(const Visibility_Token & cam_vis_token)
{
	// Update GI buffer
	{
		const auto cameraBuffer = m_enginePackage->m_Camera.getCameraBuffer();
		const vec2 &size = cameraBuffer.Dimensions;
		const float ar = size.x / size.y;
		const float tanHalfHFOV = (tanf(glm::radians(cameraBuffer.FOV / 2.0f)));
		const float tanHalfVFOV = (tanf(glm::radians((cameraBuffer.FOV / ar) / 2.0f)));
		const float points[4] = { 
			m_nearPlane * tanHalfHFOV,
			m_farPlane  * tanHalfHFOV,
			m_nearPlane * tanHalfVFOV,
			m_farPlane  * tanHalfVFOV 
		};
		const vec3 frustumCorners[8] = {
			// near face
			vec3(points[0], points[2], m_nearPlane),
			vec3(-points[0], points[2], m_nearPlane),
			vec3(points[0], -points[2], m_nearPlane),
			vec3(-points[0], -points[2], m_nearPlane),
			// far face
			vec3(points[1], points[3], m_farPlane),
			vec3(-points[1], points[3], m_farPlane),
			vec3(points[1], -points[3], m_farPlane),
			vec3(-points[1], -points[3], m_farPlane)
		};
		// Find the middle of current view frustum chunk
		const vec3 middle(0, 0, ((m_farPlane - m_nearPlane) / 2.0f) + m_nearPlane);
		// Measure distance from middle to the furthest point of frustum slice
		// Used for make a bounding sphere, but then converted into a bounding box
		const vec3 aabb(glm::length(frustumCorners[7] - middle));
		const vec3 volumeUnitSize = (aabb - -aabb) / float(m_resolution);
		const vec3 frustumpos = (cameraBuffer.vMatrix_Inverse* vec4(middle, 1.0f));
		const vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
		const vec3 newMin = -aabb + clampedPos;
		const vec3 newMax = aabb + clampedPos;
		const float l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
		m_camera.setMatrices(glm::ortho(l, r, b, t, n, f), mat4(1.0f));
		m_camera.setPosition(clampedPos);
		m_camera.setFarPlane(aabb.x);
	
		m_attribBuffer.write(0, sizeof(vec3), &newMax);
		m_attribBuffer.write(sizeof(vec4), sizeof(vec3), &newMin);
		m_attribBuffer.write(offsetof(GI_Radiance_Struct, R_wcs), sizeof(float), &volumeUnitSize.x);
	}

	// Update Draw call buffers
	for each (auto technique in *m_baseTechs)
		technique->updateDataGI(m_camera.getVisibilityToken(), m_resolution);
}

void GlobalIllumination_RH::applyPrePass(const Visibility_Token & cam_vis_token)
{
	// Generate GI from lights
	if (m_vaoLoaded) {
		// Prepare rendering state
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquationSeparatei(0, GL_MIN, GL_MIN);
		glBindVertexArray(m_quadVAO);

		m_attribBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		bindForWriting(0);
		bindNoise(4);
		m_geometryFBO->bindDepthReading(3);

		// Perform primary light bounce
		for each (auto technique in *m_baseTechs)
			technique->renderLightBounce();

		// Perform secondary light bounce
		if (m_shaderGISecondBounce->existsYet()) {
			m_IndirectSecondLayersBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			m_shaderGISecondBounce->bind();
			bindForReading(0, 5);
			bindForWriting(1);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
}

void GlobalIllumination_RH::applyLighting(const Visibility_Token & cam_vis_token)
{
	// Reconstruct GI from data
	if (m_vaoLoaded && m_shaderGIReconstruct->existsYet()) {
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		m_attribBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		m_geometryFBO->bindForReading();
		m_lightingFBO->bindForWriting();
		bindNoise(5);

		m_shaderGIReconstruct->bind();
		bindForReading(1, 6);
		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}	
}

void GlobalIllumination_RH::bindForWriting(const GLuint &bounceSpot)
{
	glViewport(0, 0, m_resolution, m_resolution);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo[bounceSpot]);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GlobalIllumination_RH::bindForReading(const GLuint &bounceSpot, const unsigned int & textureUnit)
{
	for (int x = 0; x < GI_TEXTURE_COUNT; ++x) 
		glBindTextureUnit(textureUnit + x, m_textures[bounceSpot][x]);
}

void GlobalIllumination_RH::bindNoise(const GLuint textureUnit)
{
	glBindTextureUnit(textureUnit, m_noise32);
}