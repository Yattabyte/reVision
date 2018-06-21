#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Lights\Directional_Cheap.h"


Directional_Tech_Cheap::~Directional_Tech_Cheap()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

Directional_Tech_Cheap::Directional_Tech_Cheap(Light_Buffers * lightBuffers)
{
	m_lightSSBO = &lightBuffers->m_lightDirCheapSSBO;
	m_size = 0;

	Asset_Shader::Create(m_shader_Lighting, "Base Lights\\Directional\\Light_Cheap");

	// Primitive Loading
	Asset_Primitive::Create(m_shapeQuad, "quad");
	m_quadVAOLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() {
		m_shapeQuad->updateVAO(m_quadVAO);
		m_quadVAOLoaded = true;
		GLuint data[4] = { m_shapeQuad->getSize(), 0, 0, 0 }; // count, primCount, first, reserved
		m_indirectShape = StaticBuffer(sizeof(GLuint) * 4, data);
	});
}

void Directional_Tech_Cheap::updateData(const Visibility_Token & vis_token, const int & updateQuality, const vec3 & camPos)
{	
	m_size = vis_token.specificSize("Light_Directional_Cheap");
	if (m_size && m_quadVAOLoaded) 
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &m_size);
}

void Directional_Tech_Cheap::renderLighting()
{
	if (m_size && m_shader_Lighting->existsYet() && m_quadVAOLoaded) {
		m_shader_Lighting->bind();										// Shader (directional)
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);		// SSBO light attribute array (directional)
		m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_quadVAO);									// Quad VAO
		glDrawArraysIndirect(GL_TRIANGLES, 0);							// Now draw
	}
}
