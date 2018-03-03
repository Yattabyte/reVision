#include "Systems\Graphics\FX Techniques\FXAA_Tech.h"


FXAA_Tech::~FXAA_Tech()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(Asset::FINALIZED, this);
}

FXAA_Tech::FXAA_Tech()
{
	Asset_Loader::load_asset(m_shaderFXAA, "FX\\FXAA");
	Asset_Loader::load_asset(m_shapeQuad, "quad"); 
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(Asset::FINALIZED, this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); });
}

void FXAA_Tech::applyEffect()
{
	const size_t &quad_size = m_shapeQuad->getSize();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	m_shaderFXAA->bind();
	glBindVertexArray(m_quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, quad_size);

	glBindVertexArray(0);
	Asset_Shader::Release();
}
