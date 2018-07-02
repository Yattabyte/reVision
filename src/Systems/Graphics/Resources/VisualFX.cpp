#include "Systems\Graphics\Resources\VisualFX.h"
#include "Engine.h"


VisualFX::~VisualFX()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

VisualFX::VisualFX()
{
	// Default Parameters
	m_Initialized = false;
	m_quadVAOLoaded = false;
	m_quadVAO = 0;
	m_fbo_GB = 0;
}

void VisualFX::initialize(Engine * engine)
{
	if (!m_Initialized) {
		m_engine = engine;
		m_engine->createAsset(m_shapeQuad, string("quad"), true);
		// Primitive Construction
		m_quadVAOLoaded = false;
		m_quadVAO = Asset_Primitive::Generate_VAO();
		m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, 0);
		m_shapeQuad->addCallback(this, [&]() mutable {
			m_quadVAOLoaded = true;
			m_shapeQuad->updateVAO(m_quadVAO);
			const GLuint quadData[4] = { m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_quadIndirectBuffer.write(0, sizeof(GLuint) * 4, quadData);
		});

		initializeCubeFilter();
		initializeGausianBlur();

		m_Initialized = true;
	}
}

void VisualFX::initializeCubeFilter()
{
}

void VisualFX::initializeGausianBlur()
{
	glCreateFramebuffers(1, &m_fbo_GB);

	m_engine->createAsset(m_shaderGB, string("FX\\gaussianBlur"), true);
	m_engine->createAsset(m_shaderGB_A, string("FX\\gaussianBlur_Alpha"), true);
}

void VisualFX::applyGaussianBlur(const GLuint & desiredTexture, const GLuint * flipTextures, const vec2 & size, const int & amount)
{
	if (m_shaderGB->existsYet() && desiredTexture && m_shapeQuad->existsYet() && m_quadVAOLoaded) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_GB);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, flipTextures[0], 0);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT1, flipTextures[1], 0);

		// Read from desired texture, blur into this frame buffer
		bool horizontal = false;
		glBindTextureUnit(0, desiredTexture);
		glBindTextureUnit(1, flipTextures[0]);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		m_shaderGB->bind();
		m_shaderGB->Set_Uniform(0, horizontal);
		m_shaderGB->Set_Uniform(1, size);

		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Blur remainder of the times minus 1
		glBindTextureUnit(0, flipTextures[1]);
		for (int i = 1; i < amount - 1; i++) {
			horizontal = !horizontal;
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB->Set_Uniform(0, horizontal);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}

		// Last blur back into desired texture
		horizontal = !horizontal;
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, desiredTexture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		m_shaderGB->Set_Uniform(0, horizontal);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void VisualFX::applyGaussianBlur_Alpha(const GLuint & desiredTexture, const GLuint * flipTextures, const vec2 & size, const int & amount)
{
	if (m_shaderGB_A->existsYet() && desiredTexture && m_shapeQuad->existsYet() && m_quadVAOLoaded) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_GB);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, flipTextures[0], 0);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT1, flipTextures[1], 0);

		// Read from desired texture, blur into this frame buffer
		bool horizontal = false;
		glBindTextureUnit(0, desiredTexture);
		glBindTextureUnit(1, flipTextures[0]);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
		m_shaderGB_A->bind();
		m_shaderGB_A->Set_Uniform(0, horizontal);
		m_shaderGB_A->Set_Uniform(1, size);

		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Blur remainder of the times minus 1
		glBindTextureUnit(0, flipTextures[1]);
		for (int i = 1; i < amount - 1; i++) {
			horizontal = !horizontal;
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB_A->Set_Uniform(0, horizontal);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}

		// Last blur back into desired texture
		horizontal = !horizontal;
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, desiredTexture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		m_shaderGB_A->Set_Uniform(0, horizontal);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}