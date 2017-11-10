#include "Rendering\Scenes\PBR\Gbuffer.h"
#include "Managers\Config_Manager.h"
#include "Managers\Message_Manager.h"

static float screen_width = 1.0f, screen_height = 1.0f;

static void WidthChangeCallback(const float &width) {
	screen_width = width;
}
static void HeightChangeCallback(const float &height) {
	screen_height = height;
}
static void AssignTextureProperties()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

GBuffer::~GBuffer()
{
	CFG::removePreferenceCallback(CFG_ENUM::C_WINDOW_WIDTH, WidthChangeCallback);
	CFG::removePreferenceCallback(CFG_ENUM::C_WINDOW_HEIGHT, HeightChangeCallback);

	// Destroy OpenGL objects
	glDeleteTextures(GBUFFER_NUM_TEXTURES, m_textures);
	glDeleteTextures(1, &m_depth_stencil);
	glDeleteFramebuffers(1, &m_fbo);
}

GBuffer::GBuffer()
{
	CFG::addPreferenceCallback(CFG_ENUM::C_WINDOW_WIDTH, WidthChangeCallback);
	CFG::addPreferenceCallback(CFG_ENUM::C_WINDOW_HEIGHT, HeightChangeCallback);
	screen_width = CFG::getPreference(CFG_ENUM::C_WINDOW_WIDTH);
	screen_height = CFG::getPreference(CFG_ENUM::C_WINDOW_HEIGHT);

	// Create the FBO
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	// Create the gbuffer textures
	glGenTextures(GBUFFER_NUM_TEXTURES, m_textures);
	glGenTextures(1, &m_depth_stencil);

	// Diffuse Texture
	glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_IMAGE]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
	AssignTextureProperties();
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GBUFFER_TEXTURE_TYPE_IMAGE, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_IMAGE], 0);

	// View-Normal Texture
	glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_VIEWNORMAL]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
	AssignTextureProperties();
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GBUFFER_TEXTURE_TYPE_VIEWNORMAL, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_VIEWNORMAL], 0);

	// Specular Texture
	glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_SPECULAR]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
	AssignTextureProperties();
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GBUFFER_TEXTURE_TYPE_SPECULAR, GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_SPECULAR], 0);

	// Depth-stencil buffer texture
	glBindTexture(GL_TEXTURE_2D, m_depth_stencil);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screen_width, screen_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	AssignTextureProperties();
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_stencil, 0);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
		std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
		MSG::Error(FBO_INCOMPLETE, "G Buffer", errorString);
	}
}

void GBuffer::Clear()
{
	BindForWriting();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GBuffer::BindForWriting()
{
	GLenum DrawBuffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2
	};
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glDrawBuffers(GBUFFER_NUM_TEXTURES, DrawBuffers);
}

void GBuffer::BindForReading()
{
	for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textures[GBUFFER_TEXTURE_TYPE_IMAGE + i]);
	}
}

void GBuffer::End()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	// Return the borrowed depth-stencil texture
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_stencil, 0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
