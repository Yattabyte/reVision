#include "Systems\Graphics\Resources\Lights\Spot_Cheap.h"
#include "Engine.h"


Spot_Cheap_Tech::~Spot_Cheap_Tech()
{
	if (m_shapeCone.get()) m_shapeCone->removeCallback(this);
}

Spot_Cheap_Tech::Spot_Cheap_Tech(Engine * engine, Light_Buffers * lightBuffers)
{
	// Default Parameters
	m_engine = engine;
	m_lightSSBO = &lightBuffers->m_lightSpotCheapSSBO;
	m_size = 0;

	// Asset Loading
	m_engine->createAsset(m_shader_Lighting, std::string("Base Lights\\Spot\\Light_Cheap"), true);
	m_engine->createAsset(m_shapeCone, std::string("cone"), true);

	// Primitive Loading
	m_coneVAOLoaded = false;
	m_coneVAO = Asset_Primitive::Generate_VAO();
	m_indirectShape = StaticBuffer(sizeof(GLuint) * 4, 0);
	m_shapeCone->addCallback(this, [&]() mutable {
		m_coneVAOLoaded = true;
		m_shapeCone->updateVAO(m_coneVAO);
		const GLuint data = m_shapeCone->getSize();
		m_indirectShape.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
	});
}

void Spot_Cheap_Tech::updateData(const Visibility_Token & vis_token, const int & updateQuality, const glm::vec3 & camPos)
{	
	m_size = vis_token.specificSize("Light_Spot_Cheap");
	if (m_size && m_coneVAOLoaded) {
		std::vector<GLuint> visArray(m_size);
		unsigned int count = 0;
		for each (const auto &component in vis_token.getTypeList<Lighting_C>("Light_Spot_Cheap"))
			visArray[count++] = component->getBufferIndex();
		m_visShapes.write(0, sizeof(GLuint)*visArray.size(), visArray.data());
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &m_size); // update primCount (2nd param)
	}
}

void Spot_Cheap_Tech::renderLighting()
{
	if (m_size && m_shader_Lighting->existsYet() && m_coneVAOLoaded) {
		glEnable(GL_STENCIL_TEST);
		glCullFace(GL_FRONT);

		m_shader_Lighting->bind();										// Shader (spots)
		m_visShapes.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);		// SSBO light attribute array (spots)
		m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_coneVAO);									// Cone VAO

		// Draw only into depth-stencil buffer
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0, 0);
		m_shader_Lighting->Set_Uniform(0, true);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Now draw into color buffers
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		m_shader_Lighting->Set_Uniform(0, false);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
}