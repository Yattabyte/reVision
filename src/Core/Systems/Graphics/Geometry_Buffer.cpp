#include "Systems\Graphics\Geometry_Buffer.h"
#include "Systems\Message_Manager.h"
#include "Utilities\Engine_Package.h"
#include <algorithm>

class GB_WidthChangeCallback : public Callback_Container {
public:
	~GB_WidthChangeCallback() {};
	GB_WidthChangeCallback(Geometry_Buffer *gBuffer) : m_gBuffer(gBuffer) {}
	void Callback(const float &value) {
		m_gBuffer->Resize(vec2(value, m_preferenceState->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT)));
	}
private:
	Geometry_Buffer *m_gBuffer;
};
class GB_HeightChangeCallback : public Callback_Container {
public:
	~GB_HeightChangeCallback() {};
	GB_HeightChangeCallback(Geometry_Buffer *gBuffer) : m_gBuffer(gBuffer) {}
	void Callback(const float &value) {
		m_gBuffer->Resize(vec2(m_preferenceState->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH), value));
	}
private:
	Geometry_Buffer *m_gBuffer;
};

static void AssignTextureProperties()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Geometry_Buffer::~Geometry_Buffer()
{	
	m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_WINDOW_WIDTH, m_widthChangeCallback);
	m_enginePackage->RemoveCallback(PREFERENCE_ENUMS::C_WINDOW_HEIGHT, m_heightChangeCallback);

	// Destroy OpenGL objects
	glDeleteTextures(GBUFFER_NUM_TEXTURES, m_textures);
	glDeleteTextures(1, &m_depth_stencil);
	glDeleteFramebuffers(1, &m_fbo);
}

Geometry_Buffer::Geometry_Buffer(Engine_Package *package) : m_enginePackage(package)
{
	m_widthChangeCallback = new GB_WidthChangeCallback(this);
	m_heightChangeCallback = new GB_HeightChangeCallback(this);
	m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_WINDOW_WIDTH, m_widthChangeCallback);
	m_enginePackage->AddCallback(PREFERENCE_ENUMS::C_WINDOW_HEIGHT, m_heightChangeCallback);
	const float screen_width = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH);
	const float screen_height = m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT);

	m_fbo = 0;
	m_depth_stencil = 0;
	for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x)
		m_textures[x] = 0;

	// Create the FBO
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	// Create the gbuffer textures
	glGenTextures(GBUFFER_NUM_TEXTURES, m_textures);
	glGenTextures(1, &m_depth_stencil);

	for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x) {
		glBindTexture(GL_TEXTURE_2D, m_textures[x]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, screen_width, screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
		AssignTextureProperties();
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + x, GL_TEXTURE_2D, m_textures[x], 0);
	}

	// Depth-stencil buffer texture
	glBindTexture(GL_TEXTURE_2D, m_depth_stencil);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screen_width, screen_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	AssignTextureProperties();
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_stencil, 0);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) {
		std::string errorString = std::string(reinterpret_cast<char const *>(glewGetErrorString(Status)));
		MSG::Error(FBO_INCOMPLETE, "Geometry Buffer", errorString);
	}
}

void Geometry_Buffer::Clear()
{
	BindForWriting();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Geometry_Buffer::BindForWriting()
{
	GLenum DrawBuffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2
	};
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glDrawBuffers(GBUFFER_NUM_TEXTURES, DrawBuffers);
}

void Geometry_Buffer::BindForReading()
{
	for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textures[i]);
	}
}

void Geometry_Buffer::End()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	// Return the borrowed depth-stencil texture
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depth_stencil, 0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Geometry_Buffer::Resize(const vec2 &size)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

	for (int x = 0; x < GBUFFER_NUM_TEXTURES; ++x) {
		glBindTexture(GL_TEXTURE_2D, m_textures[x]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}
	
	// Depth-stencil buffer texture
	glBindTexture(GL_TEXTURE_2D, m_depth_stencil);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, size.x, size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
