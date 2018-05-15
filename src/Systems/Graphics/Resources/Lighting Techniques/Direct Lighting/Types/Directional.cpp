#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Types\Directional.h"
#include "Systems\Graphics\Resources\Frame Buffers\Shadow_FBO.h"


Directional_Tech::~Directional_Tech()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

Directional_Tech::Directional_Tech(Shadow_FBO * shadowFBO, Light_Buffers * lightBuffers)
{
	m_shadowFBO = shadowFBO;
	m_lightSSBO = &lightBuffers->m_lightDirSSBO;
	m_size = 0;

	Asset_Loader::load_asset(m_shader_Lighting, "Lighting\\Direct Lighting\\directional");
	Asset_Loader::load_asset(m_shader_Cull, "Geometry\\cullingDir");
	Asset_Loader::load_asset(m_shader_Shadow, "Geometry\\geometryShadowDir");

	// Primitive Loading
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	m_quadVAOLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() {
		m_shapeQuad->updateVAO(m_quadVAO);
		m_quadVAOLoaded = true;
		GLuint data[4] = { m_shapeQuad->getSize(), 0, 0, 0 }; // count, primCount, first, reserved
		m_indirectShape = StaticBuffer(sizeof(GLuint) * 4, data);
	});
}

void Directional_Tech::updateData(const Visibility_Token & vis_token, const int & updateQuality, const vec3 & camPos)
{	
	m_size = vis_token.specificSize("Light_Directional");
	if (m_size && m_quadVAOLoaded) {
		// Retrieve a sorted list of most important lights to run shadow calc for.
		PriorityLightList queue(updateQuality, camPos);

		for each (const auto &component in vis_token.getTypeList<Lighting_Component>("Light_Directional"))
			queue.insert(component);

		m_queue = queue.toList();
		for each (const auto &c in m_queue)
			c->update();

		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &m_size);
	}
}

void Directional_Tech::renderOcclusionCulling()
{
	if (m_size && m_shader_Cull->existsYet()) {
		m_shader_Cull->bind();
		m_shadowFBO->bindForWriting(SHADOW_LARGE);
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
		for each (const auto &c in m_queue)
			c->occlusionPass();
	}
}

void Directional_Tech::renderShadows()
{
	if (m_size && m_shader_Shadow->existsYet()) {
		m_shader_Shadow->bind();
		m_shadowFBO->bindForWriting(SHADOW_LARGE);
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
		for each (auto &component in m_queue)
			component->shadowPass();		
	}
}

void Directional_Tech::renderLighting()
{
	if (m_size && m_shader_Lighting->existsYet() && m_quadVAOLoaded) {
		m_shader_Lighting->bind();										// Shader (directional)
		m_shadowFBO->bindForReading(SHADOW_LARGE, 4);					// Shadow maps (large maps)
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);		// SSBO light attribute array (directional)
		m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_quadVAO);									// Quad VAO
		glDrawArraysIndirect(GL_TRIANGLES, 0);							// Now draw
	}
}
