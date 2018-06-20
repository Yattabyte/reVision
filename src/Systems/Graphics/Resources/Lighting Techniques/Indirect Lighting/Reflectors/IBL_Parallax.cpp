#include "Systems\Graphics\Resources\Lighting Techniques\Indirect Lighting\Reflectors\IBL_Parallax.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\World\ECS\Components\Reflector_Component.h"
#include "Systems\World\World.h"
#include "Utilities\EnginePackage.h"
#include <minmax.h>


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
	Asset_Loader::load_asset(m_shaderConvolute, "Lighting\\Indirect Lighting\\Reflections (specular)\\Cube_Convolution");
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
	GLuint quad6Data[4] = { 6, 6, 0, 0 }; // count, primCount, first, reserved
	m_quad6FaceIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, quad6Data);
	GLuint cubeData[4] = { 36, 0, 0, 0 }; // count, primCount, first, reserved
	m_cubeIndirectBuffer = StaticBuffer(sizeof(GLuint) * 4, cubeData);
	m_visRefUBO = StaticBuffer(sizeof(GLuint) * 500, 0);
	
	m_reflectorCount = 0;
	m_fbo = 0;
	m_texture = 0;
	glCreateFramebuffers(1, &m_fbo);
	glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &m_texture);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_LOD, 0);
	glTextureParameteri(m_texture, GL_TEXTURE_MAX_LOD, 5);
	glTextureParameteri(m_texture, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(m_texture, GL_TEXTURE_MAX_LEVEL, 5);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	for (int x = 0; x < 6; ++x) {
		const ivec2 size(floor(512.0f / pow(2, x)));
		glTextureImage3DEXT(m_texture, GL_TEXTURE_CUBE_MAP_ARRAY, x, GL_RGB16F, size.x, size.y, 6, 0, GL_RGB, GL_FLOAT, NULL);
	}
	glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
	glNamedFramebufferDrawBuffer(m_fbo, GL_COLOR_ATTACHMENT0);
	const GLenum Status = glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR) 
		MSG_Manager::Error(MSG_Manager::FBO_INCOMPLETE, "IBL_Parallax_Tech", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));			
}

void IBL_Parallax_Tech::addElement()
{
	m_reflectorCount++;
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_texture, GL_TEXTURE_MIN_LOD, 0);
	glTextureParameteri(m_texture, GL_TEXTURE_MAX_LOD, 5);
	glTextureParameteri(m_texture, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteri(m_texture, GL_TEXTURE_MAX_LEVEL, 5);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	for (int x = 0; x < 6; ++x) {
		const ivec2 size(floor(512.0f / pow(2, x)));
		glTextureImage3DEXT(m_texture, GL_TEXTURE_CUBE_MAP_ARRAY, x, GL_RGB16F, size.x, size.y, m_reflectorCount * 6, 0, GL_RGB, GL_FLOAT, NULL);
	}
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
	if (m_quadVAOLoaded && m_regenCubemap && m_shaderCopy->existsYet() && m_shaderConvolute->existsYet()) {
		m_regenCubemap = false;

		// Get the renderer size, and then change it to the cubemap size
		const float width = m_enginePackage->getPreference(PreferenceState::C_WINDOW_WIDTH);
		const float height = m_enginePackage->getPreference(PreferenceState::C_WINDOW_HEIGHT);
		m_enginePackage->setPreference(PreferenceState::C_WINDOW_WIDTH, 512.0f);
		m_enginePackage->setPreference(PreferenceState::C_WINDOW_HEIGHT, 512.0f);

		auto graphics = m_enginePackage->getSubSystem<System_Graphics>("Graphics");
		vector<Reflector_Component*> listCopy = m_refList;
		for each (const auto & component in listCopy) {
			const int componentIndex = component->getBufferIndex();

			// Render 6 faces
			for (int x = 0; x < 6; ++x) {
				// Render frame from reflector's perspective
				component->bindCamera(x);
				graphics->update(0.0f);

				// Copy lighting frame into cube-face
				graphics->m_lightingFBO.bindForReading();
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
				m_shaderCopy->bind();
				m_shaderCopy->Set_Uniform(0, x + (componentIndex*6));
				glBindVertexArray(m_quadVAO);
				m_quadIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
				glDrawArraysIndirect(GL_TRIANGLES, 0);
			}

			// Once cubemap is generated, convolute it
			m_shaderConvolute->bind();
			m_shaderConvolute->Set_Uniform(0, componentIndex);
			glBindTextureUnit(0, m_texture);
			m_quad6FaceIndirectBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			for (float r = 1; r < 6; ++r) {
				// Ensure we are writing to MIP level r
				const float write_size = max(1.0f, (floor(512.0f / pow(2, r))));
				glViewport(0, 0, write_size, write_size);
				m_shaderConvolute->Set_Uniform(1, r /5.0f);
				glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, r);

				// Ensure we are reading from MIP level r - 1
				glTextureParameterf(m_texture, GL_TEXTURE_BASE_LEVEL, r - 1.0f);
				glTextureParameterf(m_texture, GL_TEXTURE_MAX_LEVEL, r - 1.0f);

				// Convolute the 6 faces for this roughness level (RENDERS 6 TIMES)
				glDrawArraysIndirect(GL_TRIANGLES, 0);
			}

			// Reset texture, so it can be used for other component reflections
			glTextureParameteri(m_texture, GL_TEXTURE_BASE_LEVEL, 0);
			glTextureParameteri(m_texture, GL_TEXTURE_MAX_LEVEL, 5);
			glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);
		}

		// Revert state, change renderer size back to previous values
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
		glClear(GL_STENCIL_BUFFER_BIT);
		
		// Draw only into depth-stencil buffer
		m_shaderEffect->Set_Uniform(0, true);
		glDisable(GL_CULL_FACE);
		glStencilFunc(GL_ALWAYS, 0, 0);
		glBlendFunc(GL_ONE, GL_ONE);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		
		// Now draw into color buffers
		m_shaderEffect->Set_Uniform(0, false);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		
		// Revert State
		glCullFace(GL_BACK);
		glDisable(GL_STENCIL_TEST);
	}
}
