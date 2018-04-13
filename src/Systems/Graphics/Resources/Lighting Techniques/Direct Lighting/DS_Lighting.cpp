#include "Systems\Graphics\Resources\Lighting Techniques\Direct Lighting\DS_Lighting.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Shadow_FBO.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"


DS_Lighting::~DS_Lighting()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	if (m_shapeCone.get()) m_shapeCone->removeCallback(this);
	if (m_shapeSphere.get()) m_shapeSphere->removeCallback(this);
}

DS_Lighting::DS_Lighting(
	Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Shadow_FBO *shadowFBO,
	VectorBuffer<Directional_Struct> * lightDirSSBO, VectorBuffer<Point_Struct> *lightPointSSBO
)
{
	// FBO's
	m_geometryFBO = geometryFBO;
	m_lightingFBO = lightingFBO;
	m_shadowFBO = shadowFBO;

	// SSBO's
	m_lightDirSSBO = lightDirSSBO;
	m_lightPointSSBO = lightPointSSBO;

	// Load Assets
	Asset_Loader::load_asset(m_shaderDirectional, "Lighting\\Direct Lighting\\directional");
	Asset_Loader::load_asset(m_shaderPoint, "Lighting\\Direct Lighting\\point");
	Asset_Loader::load_asset(m_shaderSpot, "Lighting\\Direct Lighting\\spot");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	Asset_Loader::load_asset(m_shapeCone, "cone");
	Asset_Loader::load_asset(m_shapeSphere, "sphere");
	m_quadVAOLoaded = false; 
	m_coneVAOLoaded = false;
	m_sphereVAOLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_coneVAO = Asset_Primitive::Generate_VAO();
	m_sphereVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); m_quadVAOLoaded = true;  });
	m_shapeCone->addCallback(this, [&]() { m_shapeCone->updateVAO(m_coneVAO); m_coneVAOLoaded = true;  });
	m_shapeSphere->addCallback(this, [&]() { 
		m_shapeSphere->updateVAO(m_sphereVAO); 
		m_sphereVAOLoaded = true;
		GLuint sphereData[4] = { m_shapeSphere->getSize(), 0, 0, 0 }; // count, primCount, first, reserved
		m_indirectPoint = StaticBuffer(sizeof(GLuint) * 4, sphereData);
	});

	// Draw Buffers
	GLuint quadData[4] = { 6, 1, 0, 0 }; // count, primCount, first, reserved
	m_indirectDir = StaticBuffer(sizeof(GLuint) * 4, quadData);
	m_visPoints = DynamicBuffer(sizeof(GLuint) * 10, 0);
}

void DS_Lighting::updateData(const Visibility_Token & vis_token)
{
	const GLuint dirSize = vis_token.getTypeList<Lighting_Component>("Light_Directional").size();
	const GLuint pointSize = vis_token.getTypeList<Lighting_Component>("Light_Point").size();

	if (dirSize && m_quadVAOLoaded)
		m_indirectDir.write(sizeof(GLuint), sizeof(GLuint), &dirSize);
	if (pointSize && m_sphereVAOLoaded) {
		vector<GLuint> visArray(pointSize);
		unsigned int count = 0;
		for each (const auto &component in vis_token.getTypeList<Lighting_Component>("Light_Point"))
			visArray[count++] = component->getBufferIndex();
		m_visPoints.write(0, sizeof(GLuint)*visArray.size(), visArray.data());
		m_indirectPoint.write(sizeof(GLuint), sizeof(GLuint), &pointSize);
	}
}

void DS_Lighting::applyLighting(const Visibility_Token & vis_token)
{
	const size_t &cone_size = m_shapeCone->getSize();
	const size_t &sphere_size = m_shapeSphere->getSize();
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	m_geometryFBO->bindForReading();
	m_lightingFBO->bindForWriting();

	if (vis_token.find("Light_Directional") && m_shaderDirectional->existsYet() && m_shapeQuad->existsYet() && m_quadVAOLoaded) {
		m_lightDirSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);	// SSBO light attribute array (directional)
		m_shadowFBO->bindForReading(SHADOW_LARGE, 3);					// Shadow maps (large maps)
		m_shaderDirectional->bind();									// Shader (directional)
		m_indirectDir.bindBuffer(GL_DRAW_INDIRECT_BUFFER);				// Draw call buffer
		glBindVertexArray(m_quadVAO);									// Quad VAO
		glDrawArraysIndirect(GL_TRIANGLES, 0);							// Now draw
	}

	glEnable(GL_STENCIL_TEST);
	glCullFace(GL_FRONT);
	m_shadowFBO->bindForReading(SHADOW_REGULAR, 3);

	if (vis_token.find("Light_Point") && m_shaderPoint->existsYet() && m_shapeSphere->existsYet() && m_sphereVAOLoaded) {
		m_visPoints.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_lightPointSSBO->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);	// SSBO light attribute array (points)
		m_shaderPoint->bind();											// Shader (points)
		m_indirectPoint.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_sphereVAO);									// Sphere VAO

		// Draw only into depth-stencil buffer
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0, 0);
		m_shaderPoint->Set_Uniform(0, true);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Now draw into color buffers
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF); 
		m_shaderPoint->Set_Uniform(0, false);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}

	if (vis_token.find("Light_Spot") && m_shaderSpot->existsYet() && m_shapeCone->existsYet() && m_coneVAOLoaded) {
		m_shaderSpot->bind();
		glBindVertexArray(m_coneVAO);
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Spot"))
			component->directPass(cone_size);
	}

	glCullFace(GL_BACK);
	glDisable(GL_STENCIL_TEST);
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);
}
