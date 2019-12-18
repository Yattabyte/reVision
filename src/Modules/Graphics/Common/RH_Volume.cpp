#include "Modules/Graphics/Common/RH_Volume.h"
#include "Modules/Graphics/Common/Camera.h"
#include "Engine.h"
#include <algorithm>


RH_Volume::~RH_Volume() noexcept
{
	// Update indicator
	*m_aliveIndicator = false;
	glDeleteFramebuffers(2, m_fboIDS);
	glDeleteTextures(RH_TEXTURE_COUNT, m_textureIDS[0]);
	glDeleteTextures(RH_TEXTURE_COUNT, m_textureIDS[1]);
}

RH_Volume::RH_Volume(Engine& engine) :
	m_engine(engine)
{
	// Preferences
	auto& preferences = engine.getPreferenceState();
	m_resolution = 16;
	preferences.getOrSetValue(PreferenceState::Preference::C_RH_BOUNCE_SIZE, m_resolution);
	preferences.addCallback(PreferenceState::Preference::C_RH_BOUNCE_SIZE, m_aliveIndicator, [&](const float& f) { resize(f); });

	glCreateFramebuffers(2, m_fboIDS);
	for (int bounce = 0; bounce < 2; ++bounce) {
		glCreateTextures(GL_TEXTURE_3D, RH_TEXTURE_COUNT, m_textureIDS[bounce]);
		for (int channel = 0; channel < RH_TEXTURE_COUNT; ++channel) {
			glTextureImage3DEXT(m_textureIDS[bounce][channel], GL_TEXTURE_3D, 0, GL_RGBA16F, (GLsizei)m_resolution, (GLsizei)m_resolution, (GLsizei)m_resolution, 0, GL_RGBA, GL_FLOAT, nullptr);
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

void RH_Volume::updateVolume(const Camera& camera)
{
	const glm::mat4 InverseView = camera->vMatrixInverse;
	const auto& ViewDimensions = camera->Dimensions;
	const float AspectRatio = ViewDimensions.x / ViewDimensions.y;
	const float tanHalfHFOV = glm::radians(camera->FOV) / 2.0F;
	const float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / AspectRatio);
	const float frustumSlice[2] = { camera->NearPlane, (camera->FarPlane * 0.25F) };
	const float frustumPoints[4] = {
		frustumSlice[0] * tanHalfHFOV,
		frustumSlice[1] * tanHalfHFOV,
		frustumSlice[0] * tanHalfVFOV,
		frustumSlice[1] * tanHalfVFOV
	};
	float largestCoordinate = std::max(abs(frustumSlice[0]), abs(frustumSlice[1]));
	for (const float frustumPoint : frustumPoints)
		largestCoordinate = std::max(largestCoordinate, abs(frustumPoint));
	const glm::vec3 centerOfVolume(0, 0, ((frustumSlice[1] - frustumSlice[0]) / 2.0F) + frustumSlice[0]);
	const float radius = glm::distance(glm::vec3(largestCoordinate), centerOfVolume);
	const glm::vec3 aabb(radius);
	m_unitSize = (radius - -radius) / m_resolution;
	const glm::vec3 frustumpos = (InverseView * glm::vec4(centerOfVolume, 1.0F));
	// Snap volume position to grid
	m_center = glm::floor((frustumpos + (m_unitSize / 2.0F)) / m_unitSize) * m_unitSize;
	m_min = -aabb + m_center;
	m_max = aabb + m_center;
}

void RH_Volume::resize(const float& resolution) noexcept
{
	m_resolution = resolution;
	for (auto& bounce : m_textureIDS)
		for (const unsigned int channel : bounce)
			glTextureImage3DEXT(channel, GL_TEXTURE_3D, 0, GL_RGBA16F, (GLsizei)m_resolution, (GLsizei)m_resolution, (GLsizei)m_resolution, 0, GL_RGBA, GL_FLOAT, nullptr);
}

void RH_Volume::clear() noexcept
{
	constexpr GLfloat clearColor[] = { 0.0F, 0.0F, 0.0F, 0.0F };
	for (const auto& bounce : m_fboIDS)
		for (GLint x = 0; x < RH_TEXTURE_COUNT; ++x)
			glClearNamedFramebufferfv(bounce, GL_COLOR, x, clearColor);
}

void RH_Volume::writePrimary() noexcept
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboIDS[0]);
}

void RH_Volume::readPrimary(const GLuint& binding) noexcept
{
	for (GLuint x = 0; x < RH_TEXTURE_COUNT; ++x)
		glBindTextureUnit(binding + x, m_textureIDS[0][x]);
}

void RH_Volume::writeSecondary() noexcept
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboIDS[1]);
}

void RH_Volume::readSecondary(const GLuint& binding) noexcept
{
	for (GLuint x = 0; x < RH_TEXTURE_COUNT; ++x)
		glBindTextureUnit(binding + x, m_textureIDS[1][x]);
}