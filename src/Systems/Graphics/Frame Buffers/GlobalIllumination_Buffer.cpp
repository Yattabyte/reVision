#include "Systems/Graphics/Frame Buffers/GlobalIllumination_Buffer.h"
#include "Managers\Message_Manager.h"
#include "Utilities\EnginePackage.h"
#include <algorithm>
#include <random>


GlobalIllumination_Buffer::~GlobalIllumination_Buffer()
{
	if (m_Initialized) {
		// Destroy OpenGL objects
		glDeleteTextures(GI_LIGHT_BOUNCE_COUNT * GI_TEXTURE_COUNT, m_textures[0]);
		glDeleteFramebuffers(GI_LIGHT_BOUNCE_COUNT, m_fbo);
	}
}

GlobalIllumination_Buffer::GlobalIllumination_Buffer()
{
	m_Initialized = false;
	m_nearPlane = -0.1f;
	m_farPlane = -1.0f;
	m_attribSSBO = 0;
	ZERO_MEM(m_fbo);
	ZERO_MEM(m_textures[0]);
	ZERO_MEM(m_textures[1]);
	m_noise32 = 0;
}

void GlobalIllumination_Buffer::initialize(const unsigned int & volumeSize, EnginePackage * enginePackage)
{
	m_enginePackage = enginePackage;
	m_attribBuffer = GI_Attribs_Buffer(volumeSize, 1.0F, 7.5f, 1.0f, 12);
	if (!m_Initialized) {
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
				glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, volumeSize, volumeSize, volumeSize, 0, GL_RGB, GL_FLOAT, 0);
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
			data[x] = vec3( randomFloats(generator), randomFloats(generator), randomFloats(generator) );
		
		glGenTextures(1, &m_noise32);
		glBindTexture(GL_TEXTURE_3D, m_noise32);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, 32, 32, 32, 0, GL_RGB, GL_FLOAT, &data);
		glBindTexture(GL_TEXTURE_3D, 0);

		// Pretend we have 4 cascades, and make the desired far plane only as far as the first would go
		float near_plane = m_nearPlane;
		float far_plane = - m_enginePackage->getPreference(PreferenceState::C_DRAW_DISTANCE);
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
		m_Initialized = true;
	}
}

void GlobalIllumination_Buffer::clear()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo[0]);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo[1]);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GlobalIllumination_Buffer::bindForWriting(const GLuint &bounceSpot)
{
	glViewport(0, 0, m_attribBuffer.resolution, m_attribBuffer.resolution);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo[bounceSpot]);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GlobalIllumination_Buffer::bindForReading(const GLuint &bounceSpot, const GLuint textureUnit)
{
	for (int x = 0; x < GI_TEXTURE_COUNT; ++x) {
		glActiveTexture(textureUnit + x);
		glBindTexture(GL_TEXTURE_3D, m_textures[bounceSpot][x]);
	}
}

void GlobalIllumination_Buffer::bindNoise(const GLuint textureUnit)
{
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_3D, m_noise32);
}

void GlobalIllumination_Buffer::updateData()
{
	const auto &cameraBuffer = m_enginePackage->m_Camera.getCameraBuffer();
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
	//m_attribBuffer.num_lights = (vis_token.visible_PointLights.size() + vis_token.visible_SpotLights.size());


	glBindBuffer(GL_UNIFORM_BUFFER, m_attribSSBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GI_Attribs_Buffer), &m_attribBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
