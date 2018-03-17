#include "Systems\Graphics\Lighting Techniques\IndirectDiffuse_GI_Tech.h"
#include "Systems\GraphiCS\Frame Buffers\Geometry_Buffer.h"
#include "Systems\GraphiCS\Frame Buffers\Lighting_Buffer.h"
#include "Systems\GraphiCS\Frame Buffers\Shadow_Buffer.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Systems\World\World.h"
#include "Utilities\EnginePackage.h"
#include "Managers\Message_Manager.h"
#include <algorithm>
#include "GLM\gtc\matrix_transform.hpp"
#include <random>


IndirectDiffuse_GI_Tech::~IndirectDiffuse_GI_Tech()
{
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

IndirectDiffuse_GI_Tech::IndirectDiffuse_GI_Tech(EnginePackage * enginePackage, Geometry_Buffer * gBuffer, Lighting_Buffer * lBuffer, Shadow_Buffer *sBuffer)
{
	m_enginePackage = enginePackage;
	m_gBuffer = gBuffer;
	m_lBuffer = lBuffer;
	m_sBuffer = sBuffer;
	m_nearPlane = -0.1f;
	m_farPlane = -1.0f;
	m_attribSSBO = 0;
	m_noise32 = 0;
	ZERO_MEM(m_fbo);
	ZERO_MEM(m_textures[0]);
	ZERO_MEM(m_textures[1]);

	Asset_Loader::load_asset(m_shaderDirectional_Bounce, "Lighting\\directional_bounce");
	Asset_Loader::load_asset(m_shaderPoint_Bounce, "Lighting\\point_bounce");
	Asset_Loader::load_asset(m_shaderSpot_Bounce, "Lighting\\spot_bounce");
	Asset_Loader::load_asset(m_shaderGISecondBounce, "Lighting\\gi_second_bounce");
	Asset_Loader::load_asset(m_shaderGIReconstruct, "Lighting\\gi_reconstruction");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); });
	m_attribBuffer = GI_Attribs_Buffer(16, 0.75, 100, 0.75, 12);

	if (m_enginePackage->findSubSystem("World")) {
		auto m_world = m_enginePackage->getSubSystem<System_World>("World");
		m_world->registerViewer(&m_camera);
	}

	GLuint VBO = 0;
	glGenVertexArrays(1, &m_bounceVAO);
	glBindVertexArray(m_bounceVAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLint), GLint(0), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(0, 1, GL_INT, sizeof(GLint), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO);
	glBindVertexArray(0);

	glGenFramebuffers(GI_LIGHT_BOUNCE_COUNT, m_fbo);
	glGenTextures(GI_LIGHT_BOUNCE_COUNT * GI_TEXTURE_COUNT, m_textures[0]);
	for (int bounce = 0; bounce < GI_LIGHT_BOUNCE_COUNT; ++bounce) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo[bounce]);
		for (int channel = 0; channel < GI_TEXTURE_COUNT; ++channel) {
			glBindTexture(GL_TEXTURE_3D, m_textures[bounce][channel]);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, m_attribBuffer.resolution, m_attribBuffer.resolution, m_attribBuffer.resolution, 0, GL_RGB, GL_FLOAT, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + channel, m_textures[bounce][channel], 0);
		}
		GLenum Buffers[] = { GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3
		};
		glDrawBuffers(4, Buffers);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
		std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
		MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "Lighting Buffer", errorString);

		// Delete before returning
		glDeleteTextures(GI_LIGHT_BOUNCE_COUNT * GI_TEXTURE_COUNT, m_textures[0]);
		glDeleteFramebuffers(GI_LIGHT_BOUNCE_COUNT, m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Generate Noise Texture
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine generator;
	vec3 data[32 * 32 * 32];
	for (int x = 0, total = (32 * 32 * 32); x < total; ++x)
		data[x] = vec3(randomFloats(generator), randomFloats(generator), randomFloats(generator));

	glGenTextures(1, &m_noise32);
	glBindTexture(GL_TEXTURE_3D, m_noise32);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, 32, 32, 32, 0, GL_RGB, GL_FLOAT, &data);
	glBindTexture(GL_TEXTURE_3D, 0);

	// Pretend we have 4 cascades, and make the desired far plane only as far as the first would go
	float near_plane = m_nearPlane;
	float far_plane = -m_enginePackage->getPreference(PreferenceState::C_DRAW_DISTANCE);
	float cLog = near_plane * powf((far_plane / near_plane), (1.0f / 4.0f));
	float cUni = near_plane + ((far_plane - near_plane) * 1.0f / 4.0f);
	float lambda = 0.4f;
	m_farPlane = (lambda*cLog) + ((1.0f - lambda)*cUni);

	glGenBuffers(1, &m_attribSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_attribSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GI_Attribs_Buffer), &m_attribBuffer, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_attribSSBO);
	GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	memcpy(p, &m_attribSSBO, sizeof(GI_Attribs_Buffer));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void IndirectDiffuse_GI_Tech::updateLighting(const Visibility_Token & cam_vis_token)
{
	// Prepare rendering state
	glDisable(GL_DEPTH_TEST);
	updateData();
	bindNoise(GL_TEXTURE4);
	bindForWriting(0);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquationSeparatei(0, GL_MIN, GL_MIN);

	// Perform primary light bounce
	glBindVertexArray(m_bounceVAO);
	// Bounce directional light
	m_sBuffer->BindForReading_GI(SHADOW_LARGE, GL_TEXTURE0);
	const Visibility_Token vis_token = m_camera.getVisibilityToken();
	const auto & dirList = vis_token.getTypeList<Lighting_Component>("Light_Directional");
	const auto & pointList = vis_token.getTypeList<Lighting_Component>("Light_Point");
	const auto & spotList = vis_token.getTypeList<Lighting_Component>("Light_Spot");
	const int & size = m_attribBuffer.resolution;

	if (vis_token.find("Light_Directional")) {
		m_shaderDirectional_Bounce->bind();
		for each (auto &component in dirList)
			component->indirectPass(size);
	}
	// Bounce point lights
	m_sBuffer->BindForReading_GI(SHADOW_REGULAR, GL_TEXTURE0);
	if (vis_token.find("Light_Point")) {
		m_shaderPoint_Bounce->bind();
		for each (auto &component in pointList)
			component->indirectPass(size);
	}
	// Bounce spot lights
	if (vis_token.find("Light_Spot")) {
		m_shaderSpot_Bounce->bind();
		for each (auto &component in spotList)
			component->indirectPass(size);
	}

	// Perform secondary light bounce
	m_shaderGISecondBounce->bind();
	bindForReading(0, GL_TEXTURE5);
	bindForWriting(1);
	glDrawArraysInstanced(GL_POINTS, 0, 1, size);

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	Asset_Shader::Release();
}

void IndirectDiffuse_GI_Tech::applyLighting(const Visibility_Token & vis_token)
{
	// Reconstruct GI from data
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	m_gBuffer->bindForReading();
	m_lBuffer->bindForWriting();

	m_shaderGIReconstruct->bind();
	const size_t &quad_size = m_shapeQuad->getSize();

	bindNoise(GL_TEXTURE4);
	glBindVertexArray(m_quadVAO);
	bindForReading(1, GL_TEXTURE5);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);
	
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	Asset_Shader::Release();
}


void IndirectDiffuse_GI_Tech::bindForWriting(const GLuint &bounceSpot)
{
	glViewport(0, 0, m_attribBuffer.resolution, m_attribBuffer.resolution);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo[bounceSpot]);
	glClear(GL_COLOR_BUFFER_BIT);
}

void IndirectDiffuse_GI_Tech::bindForReading(const GLuint &bounceSpot, const GLuint textureUnit)
{
	for (int x = 0; x < GI_TEXTURE_COUNT; ++x) {
		glActiveTexture(textureUnit + x);
		glBindTexture(GL_TEXTURE_3D, m_textures[bounceSpot][x]);
	}
}

void IndirectDiffuse_GI_Tech::bindNoise(const GLuint textureUnit)
{
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_3D, m_noise32);
}

void IndirectDiffuse_GI_Tech::updateData()
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

	glBindBuffer(GL_UNIFORM_BUFFER, m_attribSSBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GI_Attribs_Buffer), &m_attribBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
