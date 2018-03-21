#include "Systems\Graphics\Frame Buffers\Reflection_Buffer.h"
#include "Utilities\EnginePackage.h"


Reflection_Buffer::~Reflection_Buffer()
{
	if (m_Initialized) {
		if (m_texture != 0) glDeleteTextures(1, &m_texture);
		m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_enginePackage->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
		
		glUnmapNamedBuffer(m_reflectorUBO);
		glDeleteBuffers(1, &m_reflectorUBO);
	}
}

Reflection_Buffer::Reflection_Buffer()
{
	m_count = 0;
	m_texture = 0;
	glCreateBuffers(1, &m_reflectorUBO);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_reflectorUBO);
	glNamedBufferStorage(m_reflectorUBO, 100 * 256, 0, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	m_reflectorBuffer = glMapNamedBufferRange(m_reflectorUBO, 0, 100 * 256, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
}

void Reflection_Buffer::initialize(EnginePackage * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_renderSize.x = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(ivec2(f, m_renderSize.y)); });
		m_renderSize.y = m_enginePackage->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(ivec2(m_renderSize.x, f)); });
		Frame_Buffer::initialize();

		glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
		glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
		glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
		glNamedFramebufferDrawBuffer(m_fbo, GL_COLOR_ATTACHMENT0);
		validate();
	}
}

void Reflection_Buffer::bindForReading(const unsigned int & texture_unit)
{
	glBindTextureUnit(texture_unit, m_texture);
}

void Reflection_Buffer::bindForWriting()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_reflectorUBO);
}

void Reflection_Buffer::resize(const vec2 & size)
{
	Frame_Buffer::resize(size);

	glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, 0, GL_RGB32F, m_renderSize.x, m_renderSize.y, 0, GL_RGB, GL_FLOAT, NULL);
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
}

void * const Reflection_Buffer::addReflector(unsigned int & uboIndex)
{
	uboIndex = m_count;
	m_count++;
	return m_reflectorBuffer;
}
