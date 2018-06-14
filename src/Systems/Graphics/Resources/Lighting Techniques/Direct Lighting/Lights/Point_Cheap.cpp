#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\Lights\Point_Cheap.h"


Point_Tech_Cheap::~Point_Tech_Cheap()
{
	if (m_shapeSphere.get()) m_shapeSphere->removeCallback(this);
}

Point_Tech_Cheap::Point_Tech_Cheap(Light_Buffers * lightBuffers)
{
	m_lightSSBO = &lightBuffers->m_lightPointCheapSSBO;
	m_size = 0;

	Asset_Loader::load_asset(m_shader_Lighting, "Base Lights\\Point\\Light_Cheap");

	// Primitive Loading
	Asset_Loader::load_asset(m_shapeSphere, "sphere");
	m_sphereVAOLoaded = false;
	m_sphereVAO = Asset_Primitive::Generate_VAO();
	m_shapeSphere->addCallback(this, [&]() {
		m_shapeSphere->updateVAO(m_sphereVAO);
		m_sphereVAOLoaded = true;
		GLuint data[4] = { m_shapeSphere->getSize(), 0, 0, 0 }; // count, primCount, first, reserved
		m_indirectShape = StaticBuffer(sizeof(GLuint) * 4, data);
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
		m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &m_size);
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