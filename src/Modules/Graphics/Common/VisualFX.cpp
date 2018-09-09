#include "Modules\Graphics\Common\VisualFX.h"
#include "Engine.h"


VisualFX::~VisualFX()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	glDeleteFramebuffers(1, &m_fbo_GB);
}

void VisualFX::initialize(Engine * engine)
{
	if (!m_Initialized) {
		m_engine = engine;
		m_shapeQuad = Asset_Primitive::Create(engine, "quad");

		// Asset-Finished Callbacks
		m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4);
		m_shapeQuad->addCallback(this, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
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

	m_shaderGB = Asset_Shader::Create(m_engine, "Effects\\Gaussian Blur");
	m_shaderGB_A = Asset_Shader::Create(m_engine, "Effects\\Gaussian Blur Alpha");
}

void VisualFX::applyGaussianBlur(const GLuint & desiredTexture, const GLuint * flipTextures, const glm::vec2 & size, const int & amount)
{
	if (m_shapeQuad->existsYet() && m_shaderGB->existsYet() && desiredTexture && m_shapeQuad->existsYet()) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_GB);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, flipTextures[0], 0);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT1, flipTextures[1], 0);

		// Read from desired texture, blur into this frame buffer
		bool horizontal = false;
		glBindTextureUnit(0, desiredTexture);
		glBindTextureUnit(1, flipTextures[0]);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		m_shaderGB->bind();
		m_shaderGB->setUniform(0, horizontal);
		m_shaderGB->setUniform(1, size);

		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Blur remainder of the times minus 1
		glBindTextureUnit(0, flipTextures[1]);
		for (int i = 1; i < amount - 1; i++) {
			horizontal = !horizontal;
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB->setUniform(0, horizontal);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}

		// Last blur back into desired texture
		horizontal = !horizontal;
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, desiredTexture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		m_shaderGB->setUniform(0, horizontal);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void VisualFX::applyGaussianBlur_Alpha(const GLuint & desiredTexture, const GLuint * flipTextures, const glm::vec2 & size, const int & amount)
{
	if (m_shapeQuad->existsYet() && m_shaderGB_A->existsYet() && desiredTexture && m_shapeQuad->existsYet()) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo_GB);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, flipTextures[0], 0);
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT1, flipTextures[1], 0);

		GLfloat clearRed = 0.0f;
		glClearTexImage(flipTextures[0], 0, GL_RED, GL_FLOAT, &clearRed);
		glClearTexImage(flipTextures[1], 0, GL_RED, GL_FLOAT, &clearRed);

		// Read from desired texture, blur into this frame buffer
		bool horizontal = false;
		glBindTextureUnit(0, desiredTexture);
		glBindTextureUnit(1, flipTextures[0]);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		m_shaderGB_A->bind();
		m_shaderGB_A->setUniform(0, horizontal);
		m_shaderGB_A->setUniform(1, size);

		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Blur remainder of the times minus 1
		glBindTextureUnit(0, flipTextures[1]);
		for (int i = 1; i < amount - 1; i++) {
			horizontal = !horizontal;
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
			m_shaderGB_A->setUniform(0, horizontal);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}

		// Last blur back into desired texture
		horizontal = !horizontal;
		glNamedFramebufferTexture(m_fbo_GB, GL_COLOR_ATTACHMENT0, desiredTexture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + horizontal);
		m_shaderGB_A->setUniform(0, horizontal);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}