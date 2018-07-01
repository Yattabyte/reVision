#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflections.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Reflection_FBO.h"
#include "Systems\Graphics\Graphics.h"
#include "Engine.h"
#include "glm\gtc\matrix_transform.hpp"
#include <minmax.h>

// Begin includes for specific reflection techniques
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\Sky_Ref.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\IBL_Parallax.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\SSR.h"
// End includes for specific reflection techniques


Reflections::~Reflections()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
}

Reflections::Reflections(Engine * engine, Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Reflection_FBO * reflectionFBO)
{
	m_engine = engine;
	m_geometryFBO = geometryFBO;
	m_lightingFBO = lightingFBO;
	m_reflectionFBO = reflectionFBO;
	
	engine->createAsset(m_shaderFinal, string("Lighting\\Indirect Lighting\\Reflections (specular)\\reflections PBR"), true);
	engine->createAsset(m_brdfMap, string("brdfLUT.png"), GL_TEXTURE_2D, false, false, true);
	engine->createAsset(m_shapeQuad, string("quad"), true);

	m_quadVAOLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); m_quadVAOLoaded = true; });
	
		
	GLuint quadData[4] = { 6, 1, 0, 0 }; // count, primCount, first, reserved
	m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData);

	m_refTechs.push_back(new Sky_Ref_Tech(engine));
	m_refTechs.push_back(new IBL_Parallax_Tech(m_engine));
	m_refTechs.push_back(new SSR_Tech(m_engine, geometryFBO, lightingFBO, reflectionFBO));
	for each(auto * tech in m_refTechs)
		m_refTechMap[tech->getName()] = tech;
}

void Reflections::updateData(const Visibility_Token & vis_token)
{
	for each (auto & tech in m_refTechs)
		tech->updateData(vis_token);
}

void Reflections::applyPrePass(const Visibility_Token & vis_token)
{
	for each (auto & tech in m_refTechs)
		tech->applyPrePass();
}

void Reflections::applyLighting(const Visibility_Token & vis_token)
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

	m_reflectionFBO->bindForWriting();

	for each (auto & tech in m_refTechs)
		tech->applyEffect();

	// Read reflection colors and correct them based on surface properties
	if (m_shaderFinal->existsYet()) {
		m_shaderFinal->bind();
		m_lightingFBO->bindForWriting(); // Write back to lighting buffer
		m_reflectionFBO->bindForReading(4); // Read from final reflections
		m_brdfMap->bind(5); // BRDF LUT
		glBlendFunc(GL_ONE, GL_ONE);
		m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}

	// Revert State
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}
