#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflections.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Reflection_FBO.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\World\ECS\Components\Reflector_Component.h"
#include "Utilities\EnginePackage.h"
#include "glm\gtc\matrix_transform.hpp"
#include <minmax.h>

// Begin includes for specific reflection techniques
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\Sky_Ref.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\SSR.h"
// End includes for specific reflection techniques


Reflections::~Reflections()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	if (m_shapeCube.get()) m_shapeCube->removeCallback(this);
}

Reflections::Reflections(EnginePackage * enginePackage, Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Reflection_FBO * reflectionFBO)
{
	m_enginePackage = enginePackage;
	m_geometryFBO = geometryFBO;
	m_lightingFBO = lightingFBO;
	m_reflectionFBO = reflectionFBO;
	
	Asset_Loader::load_asset(m_shaderFinal, "Lighting\\Indirect Lighting\\Reflections (specular)\\reflections PBR");
	Asset_Loader::load_asset(m_shaderParallax, "Lighting\\Indirect Lighting\\Reflections (specular)\\parallax IBL");
	Asset_Loader::load_asset(m_brdfMap, "brdfLUT.png");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	Asset_Loader::load_asset(m_shapeCube, "box");

	m_quadVAOLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); m_quadVAOLoaded = true; });
	m_cubeVAOLoaded = false;
	m_cubeVAO = Asset_Primitive::Generate_VAO();
	m_shapeCube->addCallback(this, [&]() { m_shapeCube->updateVAO(m_cubeVAO); m_cubeVAOLoaded = true; });
		
	GLuint quadData[4] = { 6, 1, 0, 0 }; // count, primCount, first, reserved
	m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData);
	GLuint cubeData[4] = { 36, 0, 0, 0 }; // count, primCount, first, reserved
	m_cubeIndirectBuffer = StaticBuffer( sizeof(GLuint) * 4, cubeData);
	m_visRefUBO = StaticBuffer(sizeof(GLuint) * 500, 0);

	m_refTechs.push_back(new Sky_Ref_Tech(m_enginePackage));
	m_refTechs.push_back(new SSR_Tech(m_enginePackage, geometryFBO, lightingFBO, reflectionFBO));
}

void Reflections::updateData(const Visibility_Token & vis_token)
{
	const size_t r_size = vis_token.specificSize("Reflector");
	if (r_size) {
		m_refList = vis_token.getTypeList<Reflector_Component>("Reflector");
		vector<GLuint> refArray(r_size);
		unsigned int count = 0;
		for each (const auto &component in m_refList)
			refArray[count++] = component->getBufferIndex();
		m_visRefUBO.write(0, sizeof(GLuint)*refArray.size(), refArray.data());
		m_cubeIndirectBuffer.write(sizeof(GLuint), sizeof(GLuint), &r_size);
	}
}

void Reflections::applyPrePass(const Visibility_Token & vis_token)
{
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


	/*// Local Reflectors
	if (vis_token.specificSize("Reflector") && m_shaderParallax->existsYet()) {
	// Stencil out parallax reflectors
	m_shaderParallax->bind();
	m_shaderParallax->Set_Uniform(0, true);
	m_visRefUBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilFunc(GL_ALWAYS, 0, 0); // Always pass stencil test
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glBindVertexArray(m_cubeVAO);
	m_cubeIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	// Fill in stenciled region
	m_shaderParallax->Set_Uniform(0, false);
	glDisable(GL_DEPTH_TEST);
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF); // Pass test if stencil value IS NOT 0
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDrawArraysIndirect(GL_TRIANGLES, 0);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_CULL_FACE);
	}*/


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
