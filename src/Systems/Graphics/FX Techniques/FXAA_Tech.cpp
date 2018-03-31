#include "Systems\Graphics\FX Techniques\FXAA_Tech.h"


FXAA_Tech::~FXAA_Tech()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

FXAA_Tech::FXAA_Tech()
{
	Asset_Loader::load_asset(m_shaderFXAA, "FX\\FXAA");
	Asset_Loader::load_asset(m_shapeQuad, "quad"); 
	m_vaoLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO(); 
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); m_vaoLoaded = true; });
	GLuint quadData[4] = { 6, 1, 0, 0 }; // count, primCount, first, reserved
	m_quadIndirectBuffer = MappedBuffer(sizeof(GLuint) * 4, quadData);
}

void FXAA_Tech::applyEffect()
{
	if (m_vaoLoaded) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		m_shaderFXAA->bind();
		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
}
