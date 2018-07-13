#include "Systems\Graphics\Resources\Lighting Techniques\Sky Lighting\Skybox.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Engine.h"


Skybox::~Skybox()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

Skybox::Skybox(Engine * engine, Lighting_FBO * lightingFBO)
{
	// Default Parameters
	m_engine = engine;
	m_lightingFBO = lightingFBO;

	// Asset Loading
	m_engine->createAsset(m_shaderSky, std::string("skybox"), true);
	m_engine->createAsset(m_textureSky, std::string("sky\\"), true);
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

void Skybox::updateData(const Visibility_Token & vis_token)
{
}

void Skybox::applyPrePass(const Visibility_Token & vis_token)
{
}

void Skybox::applyLighting(const Visibility_Token & vis_token)
{
	// Proceed only when vao is ready (everything else is safe)
	if (m_quadVAOLoaded && m_shaderSky->existsYet()) {
		glDisable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);

		m_lightingFBO->bindForWriting();
		m_textureSky->bind(0);
		m_shaderSky->bind();
		glBindVertexArray(m_quadVAO);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
	}
}
