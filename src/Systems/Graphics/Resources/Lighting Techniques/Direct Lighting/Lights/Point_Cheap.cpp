#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Lights\Point_Cheap.h"
#include "Engine.h"


Point_Tech_Cheap::~Point_Tech_Cheap()
{
	if (m_shapeSphere.get()) m_shapeSphere->removeCallback(this);
}

Point_Tech_Cheap::Point_Tech_Cheap(Engine * engine, Light_Buffers * lightBuffers)
{
	// Default Parameters
	m_engine = engine;
	m_lightSSBO = &lightBuffers->m_lightPointCheapSSBO;
	m_size = 0;

	// Asset Loading
	m_engine->createAsset(m_shader_Lighting, string("Base Lights\\Point\\Light_Cheap"), true);
	m_engine->createAsset(m_shapeSphere, string("sphere"), true);

	// Primitive Construction
	m_sphereVAOLoaded = false;
	m_sphereVAO = Asset_Primitive::Generate_VAO();
	m_indirectShape = StaticBuffer(sizeof(GLuint) * 4, 0);
	m_shapeSphere->addCallback(this, [&]() mutable {
		m_sphereVAOLoaded = true;
		m_shapeSphere->updateVAO(m_sphereVAO);
		const GLuint data = m_shapeSphere->getSize();
		m_indirectShape.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
	});
}

void Point_Tech_Cheap::updateData(const Visibility_Token & vis_token, const int & updateQuality, const vec3 & camPos)
{	
	m_size = vis_token.specificSize("Light_Point_Cheap");
	if (m_size && m_sphereVAOLoaded) {
		vector<GLuint> visArray(m_size);
		unsigned int count = 0;
		for each (const auto &component in vis_token.getTypeList<Lighting_Component>("Light_Point_Cheap"))
			visArray[count++] = component->getBufferIndex();
		m_visShapes.write(0, sizeof(GLuint)*visArray.size(), visArray.data());
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &m_size); // update primCount (2nd param)
	}
}

void Point_Tech_Cheap::renderLighting()
{
	if (m_size && m_shader_Lighting->existsYet() && m_sphereVAOLoaded) {
		glEnable(GL_STENCIL_TEST);
		glCullFace(GL_FRONT);

		m_shader_Lighting->bind();										// Shader (points)
		m_visShapes.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_lightSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);		// SSBO light attribute array (points)
		m_indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_sphereVAO);									// Cone VAO

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