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

DS_Lighting::DS_Lighting(Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Shadow_FBO *shadowFBO)
{
	m_geometryFBO = geometryFBO;
	m_lightingFBO = lightingFBO;
	m_shadowFBO = shadowFBO;

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
	m_shapeSphere->addCallback(this, [&]() { m_shapeSphere->updateVAO(m_sphereVAO); m_sphereVAOLoaded = true;  });
}

void DS_Lighting::updateLighting(const Visibility_Token & vis_token)
{
}

void DS_Lighting::applyLighting(const Visibility_Token & vis_token)
{
	const size_t &quad_size = m_shapeQuad->getSize();
	const size_t &cone_size = m_shapeCone->getSize();
	const size_t &sphere_size = m_shapeSphere->getSize();
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	m_geometryFBO->bindForReading();
	m_lightingFBO->bindForWriting();

	m_shadowFBO->bindForReading(SHADOW_LARGE, 3);
	if (vis_token.find("Light_Directional") && m_shaderDirectional->existsYet() && m_shapeQuad->existsYet() && m_quadVAOLoaded) {
		m_shaderDirectional->bind();
		glBindVertexArray(m_quadVAO);
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Directional"))
			component->directPass(quad_size);
	}

	glEnable(GL_STENCIL_TEST);
	glCullFace(GL_FRONT);
	m_shadowFBO->bindForReading(SHADOW_REGULAR, 3);
	if (vis_token.find("Light_Point") && m_shaderPoint->existsYet() && m_shapeSphere->existsYet() && m_sphereVAOLoaded) {
		m_shaderPoint->bind();
		glBindVertexArray(m_sphereVAO);
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Point")) 
			component->directPass(sphere_size);
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
	glBindVertexArray(0);
	Asset_Shader::Release();
}
