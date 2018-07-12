#include "Systems\Graphics\Resources\Lights\Directional_Cheap.h"
#include "Engine.h"


Directional_Tech_Cheap::~Directional_Tech_Cheap()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

Directional_Tech_Cheap::Directional_Tech_Cheap(Engine * engine, Light_Buffers * lightBuffers)
{
	// Default Parameters
	m_engine = engine;
	m_lightSSBO = &lightBuffers->m_lightDirCheapSSBO;
	m_size = 0;

	// Asset Loading
	m_engine->createAsset(m_shader_Lighting, std::string("Base Lights\\Directional\\Light_Cheap"), true);
	m_engine->createAsset(m_shapeQuad, std::string("quad"), true);

	// Primitive Construction
	m_quadVAOLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_indirectShape = StaticBuffer(sizeof(GLuint) * 4, 0);
	m_shapeQuad->addCallback(this, [&]() mutable {
		m_quadVAOLoaded = true;
		m_shapeQuad->updateVAO(m_quadVAO);
		const GLuint data = m_shapeQuad->getSize();
		m_indirectShape.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
	});
}

void Directional_Tech_Cheap::updateData(const Visibility_Token & vis_token, const int & updateQuality, const glm::vec3 & camPos)
{	
	m_size = vis_token.specificSize("Light_Directional_Cheap");
	if (m_size && m_quadVAOLoaded) 
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &m_size); // update primCount (2nd param)
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
