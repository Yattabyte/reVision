#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\IBL_Parallax.h"
#include "Systems\World\ECS\Components\Reflector_Component.h"
#include "Systems\World\World.h"
#include "Utilities\EnginePackage.h"


IBL_Parallax_Tech::~IBL_Parallax_Tech()
{
	if (m_shapeCube.get()) m_shapeCube->removeCallback(this);
}

IBL_Parallax_Tech::IBL_Parallax_Tech(EnginePackage * enginePackage)
{
	// Copy Pointers
	m_enginePackage = enginePackage;

	Asset_Loader::load_asset(m_shaderEffect, "Lighting\\Indirect Lighting\\Reflections (specular)\\IBL_Parallax");
	Asset_Loader::load_asset(m_shapeCube, "box"); 
	
	m_cubeVAOLoaded = false;
	m_cubeVAO = Asset_Primitive::Generate_VAO();
	m_shapeCube->addCallback(this, [&]() { m_shapeCube->updateVAO(m_cubeVAO); m_cubeVAOLoaded = true; });


	m_regenCubemap = false;
	m_enginePackage->getSubSystem<System_World>("World")->notifyWhenLoaded(&m_regenCubemap);

	m_visRefUBO = StaticBuffer(sizeof(GLuint) * 500, 0);
	GLuint cubeData[4] = { 36, 0, 0, 0 }; // count, primCount, first, reserved
	m_cubeIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, cubeData);
}

void IBL_Parallax_Tech::updateData(const Visibility_Token & vis_token)
{
	m_size = vis_token.specificSize("Reflector");
	if (m_size) {
		m_refList = vis_token.getTypeList<Reflector_Component>("Reflector");
		vector<GLuint> refArray(m_size);
		unsigned int count = 0;
		for each (const auto &component in m_refList)
			refArray[count++] = component->getBufferIndex();
		m_visRefUBO.write(0, sizeof(GLuint)*refArray.size(), refArray.data());
		m_cubeIndirectBuffer.write(sizeof(GLuint), sizeof(GLuint), &m_size);
	}
}

void IBL_Parallax_Tech::applyPrePass()
{
	if (m_regenCubemap) {
		m_regenCubemap = false;
	}
}

void IBL_Parallax_Tech::applyEffect()
{
	// Local Reflectors
	if (m_cubeVAOLoaded && m_size && m_shaderEffect->existsYet()) {
		// Stencil out parallax reflectors
		m_shaderEffect->bind();
		m_shaderEffect->Set_Uniform(0, true);
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
		m_shaderEffect->Set_Uniform(0, false);
		glDisable(GL_DEPTH_TEST);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF); // Pass test if stencil value IS NOT 0
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glDisable(GL_STENCIL_TEST);
		glEnable(GL_CULL_FACE);
	}
}
