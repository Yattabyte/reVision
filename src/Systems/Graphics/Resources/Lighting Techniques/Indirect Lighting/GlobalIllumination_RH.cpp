#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\GlobalIllumination_RH.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Shadow_FBO.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Systems\World\World.h"
#include "Utilities\EnginePackage.h"
#include "Managers\Message_Manager.h"
#include <algorithm>
#include "GLM\gtc\matrix_transform.hpp"
#include <random>


GlobalIllumination_RH::~GlobalIllumination_RH()
{
	glUnmapNamedBuffer(m_attribSSBO);
	glDeleteBuffers(1, &m_attribSSBO);
	glDeleteTextures(1, &m_noise32);
	glDeleteTextures(GI_LIGHT_BOUNCE_COUNT * GI_TEXTURE_COUNT, m_textures[0]);
	glDeleteFramebuffers(GI_LIGHT_BOUNCE_COUNT, m_fbo);
	glDeleteVertexArrays(1, &m_bounceVAO);
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	if (m_enginePackage) {
		if (m_enginePackage->findSubSystem("World")) {
			auto m_world = m_enginePackage->getSubSystem<System_World>("World");
			m_world->unregisterViewer(&m_camera);
		}
	}
}

GlobalIllumination_RH::GlobalIllumination_RH(EnginePackage * enginePackage, Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Shadow_FBO *shadowFBO)
{
	m_enginePackage = enginePackage;
	m_geometryFBO = geometryFBO;
	m_lightingFBO = lightingFBO;
	m_shadowFBO = shadowFBO;
	m_nearPlane = -0.1f;
	m_farPlane = -1.0f;
	m_attribSSBO = 0;
	m_noise32 = 0;
	ZERO_MEM(m_fbo);
	ZERO_MEM(m_textures[0]);
	ZERO_MEM(m_textures[1]);

	Asset_Loader::load_asset(m_shaderDirectional_Bounce, "Lighting\\Indirect Lighting\\Global Illumination (diffuse)\\directional_bounce");
	Asset_Loader::load_asset(m_shaderPoint_Bounce, "Lighting\\Indirect Lighting\\Global Illumination (diffuse)\\point_bounce");
	Asset_Loader::load_asset(m_shaderSpot_Bounce, "Lighting\\Indirect Lighting\\Global Illumination (diffuse)\\spot_bounce");
	Asset_Loader::load_asset(m_shaderGISecondBounce, "Lighting\\Indirect Lighting\\Global Illumination (diffuse)\\gi_second_bounce");
	Asset_Loader::load_asset(m_shaderGIReconstruct, "Lighting\\Indirect Lighting\\Global Illumination (diffuse)\\gi_reconstruction");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); });
	m_attribBuffer = GI_Attribs_Buffer(16, 0.75, 100, 0.75, 12);

	if (m_enginePackage->findSubSystem("World")) {
		auto m_world = m_enginePackage->getSubSystem<System_World>("World");
		m_world->registerViewer(&m_camera);
	}

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
			glTextureImage3DEXT(m_textures[bounce][channel], GL_TEXTURE_3D, 0, GL_RGB32F, m_attribBuffer.resolution, m_attribBuffer.resolution, m_attribBuffer.resolution, 0, GL_RGB, GL_FLOAT, 0);
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
	glTextureImage3DEXT(m_noise32, GL_TEXTURE_3D, 0, GL_RGB32F, 32, 32, 32, 0, GL_RGB, GL_FLOAT, &data);
	glTextureParameteri(m_noise32, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTextureParameteri(m_noise32, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTextureParameteri(m_noise32, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
	glTextureParameteri(m_noise32, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_noise32, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Pretend we have 4 cascades, and make the desired far plane only as far as the first would go
	float near_plane = m_nearPlane;
	float far_plane = -m_enginePackage->getPreference(PreferenceState::C_DRAW_DISTANCE);
	float cLog = near_plane * powf((far_plane / near_plane), (1.0f / 4.0f));
	float cUni = near_plane + ((far_plane - near_plane) * 1.0f / 4.0f);
	float lambda = 0.4f;
	m_farPlane = (lambda*cLog) + ((1.0f - lambda)*cUni);

	glCreateBuffers(1, &m_attribSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_attribSSBO);
	glNamedBufferStorage(m_attribSSBO, sizeof(GI_Attribs_Buffer), &m_attribBuffer, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	m_bufferPtr = glMapNamedBufferRange(m_attribSSBO, 0, sizeof(GI_Attribs_Buffer), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

	GLuint pointData[4] = { 1, m_attribBuffer.resolution, 0, 0 }; // count, primCount, first, reserved
	m_pointsIndirectBuffer = MappedBuffer(sizeof(GLuint) * 4, pointData);
	GLuint quadData[4] = { 6, 1, 0, 0 }; // count, primCount, first, reserved
	m_quadIndirectBuffer = MappedBuffer(sizeof(GLuint) * 4, quadData);
}

void GlobalIllumination_RH::updateLighting(const Visibility_Token & cam_vis_token)
{
	// Prepare rendering state
	glDisable(GL_DEPTH_TEST);
	updateData();
	bindNoise(4);
	bindForWriting(0);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquationSeparatei(0, GL_MIN, GL_MIN);

	// Perform primary light bounce
	m_shadowFBO->BindForReading_GI(SHADOW_LARGE, 0);
	glBindVertexArray(m_bounceVAO);
	m_pointsIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	const Visibility_Token &vis_token = m_camera.getVisibilityToken();
	const auto & dirList = vis_token.getTypeList<Lighting_Component>("Light_Directional");
	const auto & pointList = vis_token.getTypeList<Lighting_Component>("Light_Point");
	const auto & spotList = vis_token.getTypeList<Lighting_Component>("Light_Spot");

	// Bounce directional lights
	if (dirList.size()) {
		m_shaderDirectional_Bounce->bind();
		for each (auto &component in dirList)
			component->indirectPass();
	}
	// Bounce point lights
	m_shadowFBO->BindForReading_GI(SHADOW_REGULAR, 0);
	if (pointList.size()) {
		m_shaderPoint_Bounce->bind();
		for each (auto &component in pointList)
			component->indirectPass();
	}
	// Bounce spot lights
	if (spotList.size()) {
		m_shaderSpot_Bounce->bind();
		for each (auto &component in spotList)
			component->indirectPass();
	}

	// Perform secondary light bounce
	m_shaderGISecondBounce->bind();
	bindForReading(0, 5);
	bindForWriting(1);
	glDrawArraysIndirect(GL_POINTS, 0);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	Asset_Shader::Release();
}

void GlobalIllumination_RH::applyLighting(const Visibility_Token & vis_token)
{
	// Reconstruct GI from data
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	m_geometryFBO->bindForReading();
	m_lightingFBO->bindForWriting();

	m_shaderGIReconstruct->bind();
	bindNoise(4);
	bindForReading(1, 5);
	glBindVertexArray(m_quadVAO);
	m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, 0);
	
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	Asset_Shader::Release();
}


void GlobalIllumination_RH::bindForWriting(const GLuint &bounceSpot)
{
	glViewport(0, 0, m_attribBuffer.resolution, m_attribBuffer.resolution);
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

void GlobalIllumination_RH::updateData()
{
	const auto cameraBuffer = m_enginePackage->m_Camera.getCameraBuffer();
	const vec2 &size = cameraBuffer.Dimensions;
	float ar = size.x / size.y;
	float tanHalfHFOV = (tanf(glm::radians(cameraBuffer.FOV / 2.0f)));
	float tanHalfVFOV = (tanf(glm::radians((cameraBuffer.FOV / ar) / 2.0f)));

	float points[4] = { m_nearPlane * tanHalfHFOV,
		m_farPlane  * tanHalfHFOV,
		m_nearPlane * tanHalfVFOV,
		m_farPlane  * tanHalfVFOV };

	vec3 frustumCorners[8] = {
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
	vec3 middle(0, 0, ((m_farPlane - m_nearPlane) / 2.0f) + m_nearPlane);

	// Measure distance from middle to the furthest point of frustum slice
	// Use to make a bounding sphere, but then convert into a bounding box
	float radius = glm::length(frustumCorners[7] - middle);
	vec3 aabb(radius);

	const vec3 volumeUnitSize = (aabb - -aabb) / float(m_attribBuffer.resolution);
	const vec3 frustumpos = (cameraBuffer.vMatrix_Inverse* vec4(middle, 1.0f));
	const vec3 clampedPos = glm::floor((frustumpos + (volumeUnitSize / 2.0f)) / volumeUnitSize) * volumeUnitSize;
	m_attribBuffer.updateBBOX(aabb, -aabb, clampedPos);
	vec3 newMin = -aabb + clampedPos;
	vec3 newMax = aabb + clampedPos;
	float l = newMin.x, r = newMax.x, b = newMax.y, t = newMin.y, n = -newMin.z, f = -newMax.z;
	m_camera.setMatrices(glm::ortho(l, r, b, t, n, f), mat4(1.0f));

	GI_Attribs_Buffer *buffer = reinterpret_cast<GI_Attribs_Buffer*>(m_bufferPtr);
	buffer = &m_attribBuffer;
}
