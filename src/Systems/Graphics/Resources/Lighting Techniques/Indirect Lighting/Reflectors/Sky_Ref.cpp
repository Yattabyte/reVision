#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\Sky_Ref.h"


Sky_Ref_Tech::~Sky_Ref_Tech()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

Sky_Ref_Tech::Sky_Ref_Tech()
{
	Asset_Shader::Create(m_shaderEffect, "Lighting\\Indirect Lighting\\Reflections (specular)\\Sky_Reflect");
	Asset_Cubemap::Create(m_textureSky, "sky\\");
	Asset_Primitive::Create(m_shapeQuad, "quad");
	
	m_quadVAOLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); m_quadVAOLoaded = true; });

	GLuint quadData[4] = { 6, 1, 0, 0 }; // count, primCount, first, reserved
	m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData);
}

void Sky_Ref_Tech::applyEffect()
{
	if (m_quadVAOLoaded && m_shaderEffect->existsYet() && m_textureSky->existsYet()) {
		m_shaderEffect->bind();
		m_textureSky->bind(4);

		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
}
