#include "Rendering\Scenes\PBR\Lighting_Buffer.h"
#include "Rendering\Scenes\PBR\Geometry_Buffer.h"
#include "Managers\Config_Manager.h"
#include "Managers\Message_Manager.h"
#include <algorithm>

static float screen_width = 1.0f, screen_height = 1.0f;
static vector<Lighting_Buffer*> lbuffers; // Exists so that we can have a single callback that will resize all the lbuffers at once

static void WidthChangeCallback(const float &width) {
	screen_width = width;
	for each (auto *buffer in lbuffers)
		buffer->Resize(vec2(screen_width, screen_height));
}

static void HeightChangeCallback(const float &height) {
	screen_height = height;
	for each (auto *buffer in lbuffers)
		buffer->Resize(vec2(screen_width, screen_height));
}


Lighting_Buffer::~Lighting_Buffer()
{
	// Remove THIS gbuffer from the static gbuffer list
	lbuffers.erase(std::remove_if(begin(lbuffers), end(lbuffers), [this](const auto *stored_buffer) {
		return (stored_buffer == this);
	}), end(lbuffers));

	CFG::removePreferenceCallback(CFG_ENUM::C_WINDOW_WIDTH, WidthChangeCallback);
	CFG::removePreferenceCallback(CFG_ENUM::C_WINDOW_HEIGHT, HeightChangeCallback);

	// Destroy OpenGL objects
	glDeleteTextures(LBUFFER_NUM_TEXTURES, m_textures);
	glDeleteFramebuffers(1, &m_fbo);
}

Lighting_Buffer::Lighting_Buffer(const GLuint &m_depth_stencil)
{
	// Add THIS lbuffer to the static gbuffer list
	lbuffers.push_back(this);

	CFG::addPreferenceCallback(CFG_ENUM::C_WINDOW_WIDTH, WidthChangeCallback);
	CFG::addPreferenceCallback(CFG_ENUM::C_WINDOW_HEIGHT, HeightChangeCallback);
	screen_width = CFG::getPreference(CFG_ENUM::C_WINDOW_WIDTH);
	screen_height = CFG::getPreference(CFG_ENUM::C_WINDOW_HEIGHT);

	this->m_depth_stencil = m_depth_stencil;
	m_fbo = 0;
	for (int x = 0; x < LBUFFER_NUM_TEXTURES; ++x)
		m_textures[x] = 0;

	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glGenTextures(LBUFFER_NUM_TEXTURES, m_textures);
	for (int x = 0; x < LBUFFER_NUM_TEXTURES; ++x) {
		glBindTexture(GL_TEXTURE_2D, m_textures[x]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screen_width, screen_height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + x, GL_TEXTURE_2D, m_textures[x], 0);
	}

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
		std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
		MSG::Error(FBO_INCOMPLETE, "Lighting Buffer", errorString);
	}
}

void Lighting_Buffer::Clear()
{
	GLenum DrawBuffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
	};

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glDrawBuffers(LBUFFER_NUM_TEXTURES, DrawBuffers);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Lighting_Buffer::BindForWriting()
{
	GLenum DrawBuffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
	};

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	// Borrow the G_Buffer's depth-stencil texture
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_stencil, 0);
	glDrawBuffers(LBUFFER_NUM_TEXTURES, DrawBuffers);
}

void Lighting_Buffer::BindForReading()
{
	for (unsigned int i = 0; i < LBUFFER_NUM_TEXTURES; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textures[i]);
	}
}

void Lighting_Buffer::Resize(const vec2 & size)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	for (int x = 0; x < LBUFFER_NUM_TEXTURES; ++x) {
		glBindTexture(GL_TEXTURE_2D, m_textures[x]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screen_width, screen_height, 0, GL_RGB, GL_FLOAT, NULL);		
	}

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
