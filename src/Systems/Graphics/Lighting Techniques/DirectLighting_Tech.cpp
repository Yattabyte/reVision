#include "Systems\Graphics\Lighting Techniques\DirectLighting_Tech.h"
#include "Systems\GraphiCS\Frame Buffers\Geometry_Buffer.h"
#include "Systems\GraphiCS\Frame Buffers\Lighting_Buffer.h"
#include "Systems\GraphiCS\Frame Buffers\Shadow_Buffer.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"


DirectLighting_Tech::~DirectLighting_Tech()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	if (m_shapeCone.get()) m_shapeCone->removeCallback(this);
	if (m_shapeSphere.get()) m_shapeSphere->removeCallback(this);
}

DirectLighting_Tech::DirectLighting_Tech(Geometry_Buffer * gBuffer, Lighting_Buffer * lBuffer, Shadow_Buffer *sBuffer)
{
	m_gBuffer = gBuffer;
	m_lBuffer = lBuffer;
	m_sBuffer = sBuffer;

	Asset_Loader::load_asset(m_shaderDirectional, "Lighting\\directional");
	Asset_Loader::load_asset(m_shaderPoint, "Lighting\\point");
	Asset_Loader::load_asset(m_shaderSpot, "Lighting\\spot");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	Asset_Loader::load_asset(m_shapeCone, "cone");
	Asset_Loader::load_asset(m_shapeSphere, "sphere");
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_coneVAO = Asset_Primitive::Generate_VAO();
	m_sphereVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); });
	m_shapeCone->addCallback(this, [&]() { m_shapeCone->updateVAO(m_coneVAO); });
	m_shapeSphere->addCallback(this, [&]() { m_shapeSphere->updateVAO(m_sphereVAO); });
}

void DirectLighting_Tech::updateLighting(const Visibility_Token & vis_token)
{
}

void DirectLighting_Tech::applyLighting(const Visibility_Token & vis_token)
{
	if (!m_shaderDirectional->existsYet()) return;
	const size_t &quad_size = m_shapeQuad->getSize();
	const size_t &cone_size = m_shapeCone->getSize();
	const size_t &sphere_size = m_shapeSphere->getSize();
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	m_gBuffer->bindForReading();
	m_lBuffer->bindForWriting();

	m_sBuffer->bindForReading(SHADOW_LARGE, 3);
	if (vis_token.find("Light_Directional") && m_shaderDirectional->existsYet()) {
		m_shaderDirectional->bind();
		glBindVertexArray(m_quadVAO);
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Directional"))
			component->directPass(quad_size);
	}

	glEnable(GL_STENCIL_TEST);
	glCullFace(GL_FRONT);
	m_sBuffer->bindForReading(SHADOW_REGULAR, 3);
	if (vis_token.find("Light_Point") && m_shaderPoint->existsYet()) {
		m_shaderPoint->bind();
		glBindVertexArray(m_sphereVAO);
		for each (auto &component in vis_token.getTypeList<Lighting_Component>("Light_Point"))
			component->directPass(sphere_size);
	}

	if (vis_token.find("Light_Spot") && m_shaderSpot->existsYet()) {
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
