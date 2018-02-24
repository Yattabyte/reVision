#include "Systems\Graphics\FX Techniques\FXAA_Tech.h"


struct Primitive_Observer : Asset_Observer
{
	Primitive_Observer(Shared_Asset_Primitive & asset, const GLuint vao) : Asset_Observer(asset.get()), m_vao_id(vao) {};
	virtual void Notify_Finalized() {
		if (m_asset->existsYet())
			dynamic_pointer_cast<Asset_Primitive>(m_asset)->updateVAO(m_vao_id);
	}
	GLuint m_vao_id;
};


FXAA_Tech::~FXAA_Tech()
{
	delete m_QuadObserver;
}

FXAA_Tech::FXAA_Tech()
{
	Asset_Loader::load_asset(m_shaderFXAA, "FX\\FXAA");
	Asset_Loader::load_asset(m_shapeQuad, "quad"); 
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_QuadObserver = (void*)(new Primitive_Observer(m_shapeQuad, m_quadVAO));
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
