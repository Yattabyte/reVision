#include "Systems\Graphics\Resources\FX Techniques\FXAA_Tech.h"
#include "Engine.h"


FXAA_Tech::~FXAA_Tech()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

FXAA_Tech::FXAA_Tech(Engine * engine)
{
	// Default Parameters
	m_engine = engine;

	// Asset Loading
	m_engine->createAsset(m_shaderFXAA, std::string("FX\\FXAA"), true);
	m_engine->createAsset(m_shapeQuad, std::string("quad"), true);

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
}

void FXAA_Tech::applyEffect()
{
	if (m_shaderFXAA->existsYet() && m_quadVAOLoaded) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		m_shaderFXAA->bind();
		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
}
