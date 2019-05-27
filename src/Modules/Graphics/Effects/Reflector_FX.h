#pragma once
#ifndef REFLECTOR_FX_H
#define REFLECTOR_FX_H 

#include "Modules/Graphics/Effects/GFX_Core_Effect.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Assets/Texture.h"
#include "Modules/Graphics/ECS/Reflector_S.h"
#include "Modules/Graphics/Common/FBO_EnvMap.h"
#include "Modules/Graphics/Common/FBO_Geometry.h"
#include "Modules/Graphics/Common/FBO_Lighting.h"
#include "Engine.h"


/** A core-rendering technique which applies parallax-corrected local cubemaps to the scene. */
class Reflector_Effect : public GFX_Core_Effect {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline ~Reflector_Effect() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Reflector_Effect(
		Engine * engine,
		FBO_Base * geometryFBO, FBO_Base * lightingFBO, FBO_Base * reflectionFBO,
		Reflector_RenderState * renderState
	) : m_engine(engine), m_geometryFBO(geometryFBO), m_lightingFBO(lightingFBO), m_reflectionFBO(reflectionFBO), m_renderState(renderState) {
		// Asset Loading
		m_shaderLighting = Shared_Shader(m_engine, "Core\\Reflector\\IBL_Parallax");
		m_shaderStencil = Shared_Shader(m_engine, "Core\\Reflector\\Stencil");
		m_shaderCopy = Shared_Shader(m_engine, "Core\\Reflector\\2D_To_Cubemap");
		m_shaderConvolute = Shared_Shader(m_engine, "Core\\Reflector\\Cube_Convolution");
		m_shapeCube = Shared_Primitive(m_engine, "cube");
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {m_renderSize = glm::ivec2(f, m_renderSize.y); });
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {m_renderSize = glm::ivec2(m_renderSize.x, f); });
		preferences.getOrSetValue(PreferenceState::C_ENVMAP_SIZE, m_renderState->m_envmapSize);
		preferences.addCallback(PreferenceState::C_ENVMAP_SIZE, m_aliveIndicator, [&](const float &f) {
			m_renderState->m_envmapSize = std::max(1u, (unsigned int)f);
			m_envmapFBO.resize(m_renderState->m_envmapSize, m_renderState->m_envmapSize, 6);
		});

		// Environment Map
		m_envmapFBO.resize(m_renderState->m_envmapSize, m_renderState->m_envmapSize, 6);

		// Asset-Finished Callbacks		
		m_shapeCube->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint data = { (GLuint)m_shapeCube->getSize() };
			m_renderState->m_indirectCube.write(0, sizeof(GLuint), &data); // count, primCount, first, reserved
		});
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_renderState->m_indirectQuad.write(0, sizeof(GLuint) * 4, quadData);
			const GLuint quad6Data[4] = { (GLuint)m_shapeQuad->getSize(), 6, 0, 0 };
			m_renderState->m_indirectQuad6Faces.write(0, sizeof(GLuint) * 4, quad6Data);
		});

		// Error Reporting
		if (glCheckNamedFramebufferStatus(m_envmapFBO.m_fboID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			m_engine->getManager_Messages().error("Reflector_FX Environment Map Framebuffer has encountered an error.");			
	}


	// Public Interface Implementation	
	inline virtual void applyEffect(const float & deltaTime) override {
		// Exit Early
		if (!m_shapeCube->existsYet() || !m_shapeQuad->existsYet() || !m_shaderLighting->existsYet() || !m_shaderStencil->existsYet() || !m_shaderCopy->existsYet() || !m_shaderConvolute->existsYet())
			return;

		// Render scene
		renderScene(deltaTime);
		// Render reflectors
		renderReflectors(deltaTime);
	}


	// Public Attributes
	VectorBuffer<Reflector_Component::GL_Buffer> m_reflectorBuffer;
	FBO_EnvMap m_envmapFBO;


protected:
	// Protected Methods
	/** Render all the geometry for each reflector */
	inline void renderScene(const float & deltaTime) {
		auto & preferences = m_engine->getPreferenceState();
		auto & graphics = m_engine->getModule_Graphics();
		bool didAnything = false, update = m_renderState->m_outOfDate;
		m_renderState->m_outOfDate = false;
		for each (auto reflector in std::vector<Reflector_Component*>(m_renderState->m_reflectorsToUpdate)) {
			if (update || reflector->m_outOfDate) {
				if (!didAnything) {
					auto copySize = m_renderSize;
					preferences.setValue(PreferenceState::C_WINDOW_WIDTH, m_renderState->m_envmapSize);
					preferences.setValue(PreferenceState::C_WINDOW_HEIGHT, m_renderState->m_envmapSize);
					glViewport(0, 0, m_renderState->m_envmapSize, m_renderState->m_envmapSize);
					m_renderSize = copySize;
					didAnything = true;
				}
				reflector->m_outOfDate = false;
				for (int x = 0; x < 6; ++x) {
					// For line below, figure out a way to copy data from the reflector data into main camera, temporarily
					// We removed the camera buffer array, as it only served this one purpose ever, and slowed everything down
					//graphics.setActiveCamera(reflector->m_Cameradata[x]->index);
					graphics.frameTick(deltaTime);

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
				reflector->m_updateTime = m_engine->getTime();
			}
		}
		if (didAnything) {
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
			Shader::Release();
			glViewport(0, 0, m_renderSize.x, m_renderSize.y);
			preferences.setValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
			preferences.setValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		}
	}
	/** Render all the lights */
	inline void renderReflectors(const float & deltaTime) {
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		// Draw only into depth-stencil buffer
		m_shaderStencil->bind();													// Shader (reflector)
		m_reflectionFBO->bindForWriting();											// Ensure writing to reflection FBO
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
		glFrontFace(GL_CW);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glFrontFace(GL_CCW);

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
	Engine * m_engine = nullptr;
	Shared_Shader m_shaderLighting, m_shaderStencil, m_shaderCopy, m_shaderConvolute;
	Shared_Primitive m_shapeCube, m_shapeQuad;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	FBO_Base * m_geometryFBO = nullptr, * m_lightingFBO = nullptr, * m_reflectionFBO = nullptr;
	Reflector_RenderState * m_renderState = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // REFLECTOR_FX_H