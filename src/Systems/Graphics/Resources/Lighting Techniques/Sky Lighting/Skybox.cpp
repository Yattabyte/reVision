#include "Systems\Graphics\Resources\Lighting Techniques\Sky Lighting\Skybox.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"


Skybox::~Skybox()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

Skybox::Skybox(Lighting_FBO * lightingFBO)
{
	m_lightingFBO = lightingFBO;
	Asset_Loader::load_asset(m_shaderSky, "skybox");
	Asset_Loader::load_asset(m_textureSky, "sky\\");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	GLuint quadData[4] = { 6, 1, 0, 0 }; // count, primCount, first, reserved
	m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData);
	m_vaoLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); m_vaoLoaded = true; });
}

void Skybox::updateData(const Visibility_Token & vis_token)
{
}

void Skybox::applyLighting(const Visibility_Token & vis_token)
{
	// Proceed only when vao is ready (everything else is safe)
	if (m_vaoLoaded) {
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
