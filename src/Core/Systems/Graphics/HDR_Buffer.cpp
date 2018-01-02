#include "Systems\Graphics\HDR_Buffer.h"
#include "Systems\Message_Manager.h"
#include "Utilities\Engine_Package.h"
#include <algorithm>

class LB_WidthChangeCallback : public Callback_Container {
public:
	~LB_WidthChangeCallback() {};
	LB_WidthChangeCallback(HDR_Buffer *lBuffer) : m_LBuffer(lBuffer) {}
	void Callback(const float &value) {
		m_LBuffer->Resize(vec2(value, m_preferenceState->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT)));
	}
private:
	HDR_Buffer *m_LBuffer;
};
class LB_HeightChangeCallback : public Callback_Container {
public:
	~LB_HeightChangeCallback() {};
	LB_HeightChangeCallback(HDR_Buffer *lBuffer) : m_LBuffer(lBuffer) {}
	void Callback(const float &value) {
		m_LBuffer->Resize(vec2(m_preferenceState->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH), value));
	}
private:
	HDR_Buffer *m_LBuffer;
};

HDR_Buffer::~HDR_Buffer()
{
	if (m_Initialized) {
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_WINDOW_WIDTH, m_widthChangeCallback);
		m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_WINDOW_HEIGHT, m_heightChangeCallback);
		delete m_widthChangeCallback;
		delete m_heightChangeCallback;

		// Destroy OpenGL objects
		glDeleteTextures(1, &m_texture);
		glDeleteFramebuffers(1, &m_fbo);
	}
}

HDR_Buffer::HDR_Buffer()
{
	m_Initialized = false;
	m_fbo = 0;
	m_texture = 0;
}

void HDR_Buffer::Initialize(Engine_Package * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_widthChangeCallback = new LB_WidthChangeCallback(this);
		m_heightChangeCallback = new LB_HeightChangeCallback(this);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_WINDOW_WIDTH, m_widthChangeCallback);
		m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_WINDOW_HEIGHT, m_heightChangeCallback);
		const float screen_width = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH);
		const float screen_height = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT);

		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, screen_width, screen_height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
			std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
			MSG::Error(FBO_INCOMPLETE, "Lighting Buffer", errorString);
			return;
		}
		m_Initialized = true;
	}
}

void HDR_Buffer::Clear()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glClear(GL_COLOR_BUFFER_BIT);
}

void HDR_Buffer::BindForWriting()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

void HDR_Buffer::BindForReading()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);	
}

void HDR_Buffer::Resize(const vec2 & size)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
