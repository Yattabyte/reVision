#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\Sky_Ref.h"
#include "Engine.h"


Sky_Ref_Tech::~Sky_Ref_Tech()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

Sky_Ref_Tech::Sky_Ref_Tech(Engine * engine)
{
	engine->createAsset(m_shaderEffect, string("Lighting\\Indirect Lighting\\Reflections (specular)\\Sky_Reflect"), true);
	engine->createAsset(m_textureSky, string("sky\\"), true);
	engine->createAsset(m_shapeQuad, string("quad"), true);
	
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
