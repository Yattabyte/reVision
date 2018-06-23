#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\SSR.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Reflection_FBO.h"
#include "Engine.h"
#include "Managers\Message_Manager.h"
#include <minmax.h>


SSR_Tech::~SSR_Tech()
{
	glDeleteTextures(1, &m_texture);
	glDeleteFramebuffers(1, &m_fbo);

	m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
	m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);

	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

SSR_Tech::SSR_Tech(Engine * engine, Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Reflection_FBO * reflectionFBO)
{
	// Copy Pointers
	m_engine = engine;
	m_geometryFBO = geometryFBO;
	m_lightingFBO = lightingFBO;
	m_reflectionFBO = reflectionFBO;

	Asset_Shader::Create(m_shaderCopy, "fx\\copyTexture");
	Asset_Shader::Create(m_shaderBlur, "fx\\gaussianBlur_MIP");
	Asset_Shader::Create(m_shaderEffect, "Lighting\\Indirect Lighting\\Reflections (specular)\\SSR");
	Asset_Primitive::Create(m_shapeQuad, "quad");

	m_renderSize.x = m_engine->addPrefCallback(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {resize(vec2(f, m_renderSize.y)); });
	m_renderSize.y = m_engine->addPrefCallback(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {resize(vec2(m_renderSize.x, f)); });

	m_quadVAOLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); m_quadVAOLoaded = true; });

	m_fbo = 0;
	glCreateFramebuffers(1, &m_fbo);
	m_texture = 0;
	glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_LOD, 0);
	glTextureParameteri(m_texture, GL_TEXTURE_MAX_LOD, 5);
	glTextureParameteri(m_texture, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(m_texture, GL_TEXTURE_MAX_LEVEL, 5);
	for (int x = 0; x < 6; ++x) {
		const ivec2 size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
		glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, x, GL_RGB16F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	glNamedFramebufferDrawBuffer(m_fbo, GL_COLOR_ATTACHMENT0);
	GLenum Status = glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) 
		MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "Lighting Buffer", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));	
	
	GLuint quadData[4] = { 6, 1, 0, 0 }; // count, primCount, first, reserved
	m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData);

	SSR_Buffer buffer;
	m_ssrBuffer = StaticBuffer(sizeof(SSR_Buffer), &buffer);
}

void SSR_Tech::applyEffect()
{
	if (m_quadVAOLoaded && m_shaderEffect->existsYet()) {
		updateMipChain();

		m_shaderEffect->bind();
		m_reflectionFBO->bindForWriting();
		m_geometryFBO->bindForReading();
		glBindTextureUnit(6, m_texture); // Blurred light MIP-chain
		m_ssrBuffer.bindBufferBase(GL_UNIFORM_BUFFER, 6);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
}

void SSR_Tech::resize(const ivec2 & size)
{
	m_renderSize = size;
	for (int x = 0; x < 6; ++x) {
		const ivec2 size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
		glTextureImage2DEXT(m_texture, GL_TEXTURE_2D, x, GL_RGB16F, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
	}
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	glNamedFramebufferDrawBuffer(m_fbo, GL_COLOR_ATTACHMENT0);
}

void SSR_Tech::updateMipChain()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// Copy lighting texture to one with a MIP chain
	if (m_shaderCopy->existsYet()) {
		m_shaderCopy->bind();
		m_lightingFBO->bindForReading();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}

	// Blur MIP chain, reading from 1 MIP level and writing into next
	if (m_shaderBlur->existsYet()) {
		m_shaderBlur->bind();
		glBindTextureUnit(0, m_texture);
		for (int horizontal = 0; horizontal < 2; ++horizontal) {
			m_shaderBlur->Set_Uniform(0, horizontal);
			ivec2 read_size = m_renderSize;
			for (int x = 1; x < 6; ++x) {
				// Ensure we are reading from MIP level x - 1
				m_shaderBlur->Set_Uniform(1, read_size);
				glTextureParameteri(m_texture, GL_TEXTURE_BASE_LEVEL, x - 1);
				glTextureParameteri(m_texture, GL_TEXTURE_MAX_LEVEL, x - 1);
				// Ensure we are writing to MIP level x
				ivec2 write_size(floor(m_renderSize.x / pow(2, x)), floor(m_renderSize.y / pow(2, x)));
				glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, x);

				glViewport(0, 0, max(1.0f, write_size.x), max(1.0f, write_size.y));
				glDrawArraysIndirect(GL_TRIANGLES, 0);
				read_size = write_size;
			}
			// Blend second pass
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
		}
	}

	// Restore to default
	glTextureParameteri(m_texture, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(m_texture, GL_TEXTURE_MAX_LEVEL, 5);
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
}
