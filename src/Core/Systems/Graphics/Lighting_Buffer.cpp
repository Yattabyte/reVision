#include "Systems\Graphics\Lighting_Buffer.h"
#include "Systems\Message_Manager.h"
#include "Utilities\Engine_Package.h"
#include <algorithm>

class LB_WidthChangeCallback : public Callback_Container {
public:
	~LB_WidthChangeCallback() {};
	LB_WidthChangeCallback(Lighting_Buffer *lBuffer) : m_LBuffer(lBuffer) {}
	void Callback(const float &value) {
		m_LBuffer->Resize(vec2(value, m_preferenceState->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT)));
	}
private:
	Lighting_Buffer *m_LBuffer;
};
class LB_HeightChangeCallback : public Callback_Container {
public:
	~LB_HeightChangeCallback() {};
	LB_HeightChangeCallback(Lighting_Buffer *lBuffer) : m_LBuffer(lBuffer) {}
	void Callback(const float &value) {
		m_LBuffer->Resize(vec2(m_preferenceState->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH), value));
	}
private:
	Lighting_Buffer *m_LBuffer;
};

Lighting_Buffer::~Lighting_Buffer()
{
	if (m_Initialized) {
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_WINDOW_WIDTH, m_widthChangeCallback);
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_WINDOW_HEIGHT, m_heightChangeCallback);
		delete m_widthChangeCallback;
		delete m_heightChangeCallback;

		// Destroy OpenGL objects
		glDeleteTextures(LBUFFER_NUM_TEXTURES, m_textures);
		glDeleteFramebuffers(1, &m_fbo);
	}
}

Lighting_Buffer::Lighting_Buffer()
{
	m_Initialized = false;
	m_depth_stencil = 0;
	m_fbo = 0;
	for (int x = 0; x < LBUFFER_NUM_TEXTURES; ++x)
		m_textures[x] = 0;
}

void Lighting_Buffer::Initialize(Engine_Package * enginePackage, const GLuint &depthStencil)
{
	if (!m_Initialized) {
		m_depth_stencil = depthStencil;
		m_enginePackage = enginePackage;
		m_widthChangeCallback = new LB_WidthChangeCallback(this);
		m_heightChangeCallback = new LB_HeightChangeCallback(this);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_WINDOW_WIDTH, m_widthChangeCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_WINDOW_HEIGHT, m_heightChangeCallback);
		const float screen_width = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH);
		const float screen_height = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT);

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
			return;
		}
		m_Initialized = true;
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
