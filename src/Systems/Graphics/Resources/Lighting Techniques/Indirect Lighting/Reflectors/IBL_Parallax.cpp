#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\IBL_Parallax.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\World\ECS\Components\Reflector_Component.h"
#include "Systems\World\World.h"
#include "Utilities\EnginePackage.h"


IBL_Parallax_Tech::~IBL_Parallax_Tech()
{
	if (m_shapeQuad.get()) m_shapeQuad->removeCallback(this);
	if (m_shapeCube.get()) m_shapeCube->removeCallback(this);
}

IBL_Parallax_Tech::IBL_Parallax_Tech(EnginePackage * enginePackage)
{
	// Copy Pointers
	m_enginePackage = enginePackage;

	Asset_Loader::load_asset(m_shaderEffect, "Lighting\\Indirect Lighting\\Reflections (specular)\\IBL_Parallax");
	Asset_Loader::load_asset(m_shaderCopy, "Utilities\\2D_To_Cubemap");
	Asset_Loader::load_asset(m_shapeQuad, "quad");
	Asset_Loader::load_asset(m_shapeCube, "box"); 
	
	m_quadVAOLoaded = false;
	m_quadVAO = Asset_Primitive::Generate_VAO();
	m_shapeQuad->addCallback(this, [&]() { m_shapeQuad->updateVAO(m_quadVAO); m_quadVAOLoaded = true; });
	m_cubeVAOLoaded = false;
	m_cubeVAO = Asset_Primitive::Generate_VAO();
	m_shapeCube->addCallback(this, [&]() { m_shapeCube->updateVAO(m_cubeVAO); m_cubeVAOLoaded = true; });

	m_regenCubemap = false;
	m_enginePackage->getSubSystem<System_World>("World")->notifyWhenLoaded(&m_regenCubemap);

	GLuint quadData[4] = { 6, 1, 0, 0 }; // count, primCount, first, reserved
	m_quadIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quadData);
	GLuint cubeData[4] = { 36, 0, 0, 0 }; // count, primCount, first, reserved
	m_cubeIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, cubeData);
	m_visRefUBO = StaticBuffer(sizeof(GLuint) * 500, 0);
	
	m_reflectorCount = 0;
	m_fbo = 0;
	m_texture = 0;
	glCreateFramebuffers(1, &m_fbo);
	glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &m_texture);
	glTextureImage3DEXT(m_texture, GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB16F, 512.0f, 512.0f, 6, 0, GL_RGB, GL_FLOAT, NULL);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	glNamedFramebufferDrawBuffer(m_fbo, GL_COLOR_ATTACHMENT0);
	const GLenum Status = glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) 
		MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "IBL_Parallax_Tech", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));			
}

void IBL_Parallax_Tech::addElement()
{
	m_reflectorCount++;
	glTextureImage3DEXT(m_texture, GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB16F, 512.0f, 512.0f, m_reflectorCount * 6, 0, GL_RGB, GL_FLOAT, NULL);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void IBL_Parallax_Tech::removeElement(const unsigned int & index)
{
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
	if (m_quadVAOLoaded && m_regenCubemap && m_shaderCopy->existsYet()) {
		m_regenCubemap = false;
		float width = m_enginePackage->getPreference(PreferenceState::C_WINDOW_WIDTH);
		float height = m_enginePackage->getPreference(PreferenceState::C_WINDOW_HEIGHT);
		m_enginePackage->setPreference(PreferenceState::C_WINDOW_WIDTH, 512.0f);
		m_enginePackage->setPreference(PreferenceState::C_WINDOW_HEIGHT, 512.0f);

		auto graphics = m_enginePackage->getSubSystem<System_Graphics>("Graphics");
		vector<Reflector_Component*> listCopy = m_refList;
		for each (const auto & component in listCopy) {
			const int componentIndex = component->getBufferIndex() * 6;
			for (int x = 0; x < 6; ++x) {
				component->bindCamera(x);
				graphics->update(0.0f);
				graphics->m_lightingFBO.bindForReading();
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
				m_shaderCopy->bind();
				m_shaderCopy->Set_Uniform(0, x + componentIndex);
				glBindVertexArray(m_quadVAO);
				m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
				glDrawArraysIndirect(GL_TRIANGLES, 0);
			}
		}
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
		Asset_Shader::Release();
		m_enginePackage->m_Camera.bind();

		m_enginePackage->setPreference(PreferenceState::C_WINDOW_WIDTH, width);
		m_enginePackage->setPreference(PreferenceState::C_WINDOW_HEIGHT, height);
	}
}

void IBL_Parallax_Tech::applyEffect()
{
	// Local Reflectors
	if (m_cubeVAOLoaded && m_size && m_shaderEffect->existsYet()) {
		//A Set up state
		m_shaderEffect->bind();
		m_visRefUBO.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_cubeIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glBindVertexArray(m_cubeVAO);
		glBindTextureUnit(4, m_texture);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		
		// Draw only into depth-stencil buffer
		m_shaderEffect->Set_Uniform(0, true);
		glClear(GL_STENCIL_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		glStencilFunc(GL_ALWAYS, 0, 0);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glBlendFunc(GL_ONE, GL_ONE);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		
		// Now draw into color buffers
		m_shaderEffect->Set_Uniform(0, false);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		
		// Revert State
		glCullFace(GL_BACK);
		glDisable(GL_STENCIL_TEST);
	}
}
