#pragma once
#ifndef REFLECTOR_FX_H
#define REFLECTOR_FX_H 

#include "Modules\Graphics\Effects\Effect_Base.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Texture.h"
#include "ECS\Systems\Reflector_S.h"
#include "Modules\Graphics\Common\FBO_EnvMap.h"
#include "Modules\Graphics\Common\FBO_Geometry.h"
#include "Modules\Graphics\Common\FBO_Lighting.h"
#include "Engine.h"
#include "GLFW\glfw3.h"


/** A core rendering effect which applies parallax-corrected local cubemaps to the scene. */
class Reflector_Effect : public Effect_Base {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Reflector_Effect() {
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_WIDTH, this);
		m_engine->removePrefCallback(PreferenceState::C_WINDOW_HEIGHT, this);
		m_engine->removePrefCallback(PreferenceState::C_ENVMAP_SIZE, this);
		if (m_shapeCube.get()) m_shapeCube->removeCallback(this);
	}
	/** Constructor. */
	Reflector_Effect(
		Engine * engine,
		FBO_Base * geometryFBO, FBO_Base * lightingFBO, FBO_Base * reflectionFBO,
		Reflector_RenderState * renderState
	) : m_engine(engine), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO), m_reflectionFBO(reflectionFBO), m_renderState(renderState) {
		// Asset Loading
		m_shaderLighting = Asset_Shader::Create(m_engine, "Core\\Reflector\\IBL_Parallax");
		m_shaderStencil = Asset_Shader::Create(m_engine, "Core\\Reflector\\Stencil");
		m_shaderCopy = Asset_Shader::Create(m_engine, "Core\\Reflector\\2D_To_Cubemap");
		m_shaderConvolute = Asset_Shader::Create(m_engine, "Core\\Reflector\\Cube_Convolution");
		m_shapeCube = Asset_Primitive::Create(m_engine, "cube");
		m_shapeQuad = Asset_Primitive::Create(m_engine, "quad");

		// Preference Callbacks
		m_renderSize.x = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_WIDTH, this, [&](const float &f) {
			m_renderSize = glm::ivec2(f, m_renderSize.y);
		});
		m_renderSize.y = m_engine->addPrefCallback<int>(PreferenceState::C_WINDOW_HEIGHT, this, [&](const float &f) {
			m_renderSize = glm::ivec2(m_renderSize.x, f);
		});
		m_renderState->m_envmapSize = m_engine->addPrefCallback<unsigned int>(PreferenceState::C_ENVMAP_SIZE, this, [&](const float &f) {
			m_renderState->m_envmapSize = std::max(1u, (unsigned int)f);
		});

		// Environment Map
		m_envmapFBO.resize(m_renderState->m_envmapSize, m_renderState->m_envmapSize, 6);

		// Asset-Finished Callbacks		
		m_shapeCube->addCallback(this, [&]() mutable {
			const GLuint data = { (GLuint)m_shapeCube->getSize() };
			m_renderState->m_indirectCube.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
		});
		m_shapeQuad->addCallback(this, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_renderState->m_indirectQuad.write(0, sizeof(GLuint) * 4, quadData);
			const GLuint quad6Data[4] = { (GLuint)m_shapeQuad->getSize(), 6, 0, 0 };
			m_renderState->m_indirectQuad6Faces.write(0, sizeof(GLuint) * 4, quad6Data);
		});

		// Error Reporting
		const GLenum Status = glCheckNamedFramebufferStatus(m_envmapFBO.m_fboID, GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
			m_engine->reportError(MessageManager::FBO_INCOMPLETE, "Reflector Environment map FBO", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
	}


	// Interface Implementation	
	virtual void applyEffect(const float & deltaTime) override {
		// Exit Early
		if (!m_shapeCube->existsYet() || !m_shapeQuad->existsYet() || !m_shaderLighting->existsYet() || !m_shaderStencil->existsYet() || !m_shaderCopy->existsYet() || !m_shaderConvolute->existsYet())
			return;

		// Render scene
		renderScene(deltaTime);
		// Render reflectors
		renderReflectors(deltaTime);
	}


	// Public Attributes
	VectorBuffer<Reflection_Buffer> m_reflectorBuffer;
	FBO_EnvMap m_envmapFBO;


protected:
	// Protected Methods
	/** Render all the geometry for each reflector */
	void renderScene(const float & deltaTime) {
		auto & graphics = m_engine->getGraphicsModule();
		bool didAnything = false, update = m_renderState->m_outOfDate;
		m_renderState->m_outOfDate = false;
		GLuint oldCameraID = 0;
		for each (auto reflector in std::vector<Reflector_Component*>(m_renderState->m_reflectorsToUpdate)) {
			if (update || reflector->m_outOfDate) {
				if (!didAnything) {
					auto copySize = m_renderSize;
					m_engine->setPreference(PreferenceState::C_WINDOW_WIDTH, m_renderState->m_envmapSize);
					m_engine->setPreference(PreferenceState::C_WINDOW_HEIGHT, m_renderState->m_envmapSize);
					glViewport(0, 0, m_renderState->m_envmapSize, m_renderState->m_envmapSize);
					m_renderSize = copySize;
					oldCameraID = graphics.getActiveCamera();
					didAnything = true;
				}
				reflector->m_outOfDate = false;
				for (int x = 0; x < 6; ++x) {
					graphics.setActiveCamera(reflector->m_Cameradata[x]->index);
					graphics.renderFrame(deltaTime);

					// Copy lighting frame into cube-face
					m_lightingFBO->bindForReading();
					m_envmapFBO.bindForWriting();
					m_shaderCopy->bind();
					m_shaderCopy->setUniform(0, x + reflector->m_cubeSpot);
					m_renderState->m_indirectQuad.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
					glBindVertexArray(m_shapeQuad->m_vaoID);
					glDrawArraysIndirect(GL_TRIANGLES, 0);
				}
				// Once cubemap is generated, convolute it
				m_shaderConvolute->bind();
				m_shaderConvolute->setUniform(0, reflector->m_cubeSpot);
				m_envmapFBO.bindForReading();
				m_renderState->m_indirectQuad6Faces.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
				for (unsigned int r = 1; r < 6; ++r) {
					// Ensure we are writing to MIP level r
					const unsigned int write_size = (unsigned int)std::max(1.0f, (floor(m_renderState->m_envmapSize / pow(2.0f, (float)r))));
					glViewport(0, 0, write_size, write_size);
					m_shaderConvolute->setUniform(1, (float)r / 5.0f);
					glNamedFramebufferTexture(m_envmapFBO.m_fboID, GL_COLOR_ATTACHMENT0, m_envmapFBO.m_textureID, r);

					// Ensure we are reading from MIP level r - 1
					glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_BASE_LEVEL, r - 1);
					glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_MAX_LEVEL, r - 1);

					// Convolute the 6 faces for this roughness level (RENDERS 6 TIMES)
					glDrawArraysIndirect(GL_TRIANGLES, 0);
				}

				// Reset texture, so it can be used for other component reflections
				glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_BASE_LEVEL, 0);
				glTextureParameteri(m_envmapFBO.m_textureID, GL_TEXTURE_MAX_LEVEL, 5);
				glNamedFramebufferTexture(m_envmapFBO.m_fboID, GL_COLOR_ATTACHMENT0, m_envmapFBO.m_textureID, 0);
				reflector->m_updateTime = (float)glfwGetTime();
			}
		}
		if (didAnything) {
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
			Asset_Shader::Release();
			glViewport(0, 0, m_renderSize.x, m_renderSize.y);
			graphics.setActiveCamera(oldCameraID);
			m_engine->setPreference(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
			m_engine->setPreference(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		}
	}
	/** Render all the lights */
	void renderReflectors(const float & deltaTime) {
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		// Draw only into depth-stencil buffer
		m_shaderStencil->bind();													// Shader (reflector)
		m_reflectionFBO->bindForWriting();											// Ensure writing to lighting FBO
		m_geometryFBO->bindForReading();											// Read from Geometry FBO
		glBindTextureUnit(4, m_envmapFBO.m_textureID);								// Reflection map (environment texture)
		m_renderState->m_visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
		m_reflectorBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);				// Reflection buffer
		m_renderState->m_indirectCube.bindBuffer(GL_DRAW_INDIRECT_BUFFER);			// Draw call buffer
		glBindVertexArray(m_shapeCube->m_vaoID);								 	// Quad VAO
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0, 0);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Now draw into color buffers
		m_shaderLighting->bind();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glCullFace(GL_FRONT);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glDepthMask(GL_TRUE);
		glCullFace(GL_BACK);
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_STENCIL_TEST);
	}


private:
	// Private Attributes
	Engine * m_engine;
	Shared_Asset_Shader m_shaderLighting, m_shaderStencil, m_shaderCopy, m_shaderConvolute;
	Shared_Asset_Primitive m_shapeCube, m_shapeQuad;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	FBO_Base * m_geometryFBO, *m_lightingFBO, *m_reflectionFBO;
	Reflector_RenderState * m_renderState;
};

#endif // REFLECTOR_FX_H