#include "Modules/Graphics/Common/RH_Volume.h"
#include "Engine.h"
#include <algorithm>


RH_Volume::~RH_Volume() 
{
	// Update indicator
	m_aliveIndicator = false;
	glDeleteFramebuffers(2, m_fboIDS);
	glDeleteTextures(RH_TEXTURE_COUNT, m_textureIDS[0]);
	glDeleteTextures(RH_TEXTURE_COUNT, m_textureIDS[1]);
}

RH_Volume::RH_Volume(Engine * engine) 
	: m_engine(engine)
{
	// Preferences
	auto & preferences = m_engine->getPreferenceState();
	m_resolution = 16;
	preferences.getOrSetValue(PreferenceState::C_RH_BOUNCE_SIZE, m_resolution);
	preferences.addCallback(PreferenceState::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float &f) { resize(f); });

	glCreateFramebuffers(2, m_fboIDS);
	for (int bounce = 0; bounce < 2; ++bounce) {
		glCreateTextures(GL_TEXTURE_3D, RH_TEXTURE_COUNT, m_textureIDS[bounce]);
		for (int channel = 0; channel < RH_TEXTURE_COUNT; ++channel) {
			glTextureImage3DEXT(m_textureIDS[bounce][channel], GL_TEXTURE_3D, 0, GL_RGBA16F, m_resolution, m_resolution, m_resolution, 0, GL_RGBA, GL_FLOAT, 0);
			glTextureParameteri(m_textureIDS[bounce][channel], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textureIDS[bounce][channel], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textureIDS[bounce][channel], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textureIDS[bounce][channel], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(m_textureIDS[bounce][channel], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glNamedFramebufferTexture(m_fboIDS[bounce], GL_COLOR_ATTACHMENT0 + channel, m_textureIDS[bounce][channel], 0);
		}
		const GLenum Buffers[] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3
		};
		glNamedFramebufferDrawBuffers(m_fboIDS[bounce], 4, Buffers);
	}
}

void RH_Volume::updateVolume(const std::shared_ptr<CameraBuffer> & cameraBuffer)
{
	const glm::mat4 InverseView = glm::inverse((*cameraBuffer)->vMatrix);
	const glm::vec2 ViewDimensions = (*cameraBuffer)->Dimensions;
	const float AspectRatio = ViewDimensions.x / ViewDimensions.y;
	const float tanHalfHFOV = glm::radians((*cameraBuffer)->FOV) / 2.0f;
	const float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / AspectRatio);
	const float frustumSlice[2] = { (*cameraBuffer)->ConstNearPlane, ((*cameraBuffer)->FarPlane * 0.25f) };
	const float frustumPoints[4] = {
		frustumSlice[0] * tanHalfHFOV,
		frustumSlice[1] * tanHalfHFOV,
		frustumSlice[0] * tanHalfVFOV,
		frustumSlice[1] * tanHalfVFOV
	};
	float largestCoordinate = std::max(abs(frustumSlice[0]), abs(frustumSlice[1]));
	for (int x = 0; x < 4; ++x)
		largestCoordinate = std::max(largestCoordinate, abs(frustumPoints[x]));
	const glm::vec3 centerOfVolume(0, 0, ((frustumSlice[1] - frustumSlice[0]) / 2.0f) + frustumSlice[0]);
	const float radius = glm::distance(glm::vec3(largestCoordinate), centerOfVolume);
	const glm::vec3 aabb(radius);
	m_unitSize = (radius - -radius) / m_resolution;
	const glm::vec3 frustumpos = (InverseView * glm::vec4(centerOfVolume, 1.0f));
	// Snap volume position to grid
	m_center = glm::floor((frustumpos + (m_unitSize / 2.0f)) / m_unitSize) * m_unitSize;
	m_min = -aabb + m_center;
	m_max = aabb + m_center;
}

void RH_Volume::resize(const float & resolution)
{
	m_resolution = resolution;
	for (int bounce = 0; bounce < 2; ++bounce)
		for (int channel = 0; channel < RH_TEXTURE_COUNT; ++channel)
			glTextureImage3DEXT(m_textureIDS[bounce][channel], GL_TEXTURE_3D, 0, GL_RGBA16F, m_resolution, m_resolution, m_resolution, 0, GL_RGBA, GL_FLOAT, 0);
}

void RH_Volume::clear()
{
	GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	for (int bounce = 0; bounce < 2; ++bounce) 
		for (GLint x = 0; x < RH_TEXTURE_COUNT; ++x)
			glClearNamedFramebufferfv(m_fboIDS[bounce], GL_COLOR, x, clearColor);
}

void RH_Volume::writePrimary()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboIDS[0]);
}

void RH_Volume::readPrimary(const GLuint & binding) 
{
	for (GLuint x = 0; x < RH_TEXTURE_COUNT; ++x)
		glBindTextureUnit(binding + x, m_textureIDS[0][x]);
}

void RH_Volume::writeSecondary()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboIDS[1]);
}

void RH_Volume::readSecondary(const GLuint & binding)
{
	for (GLuint x = 0; x < RH_TEXTURE_COUNT; ++x)
		glBindTextureUnit(binding + x, m_textureIDS[1][x]);
}