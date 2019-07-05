#pragma once
#ifndef REFLECTOR_TECHNIQUE_H
#define REFLECTOR_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorVisibility_System.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorScheduler_System.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorSync_System.h"
#include "Modules/World/ECS/components.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Engine.h"


/** A core lighting technique responsible for all parallax reflectors. */
class Reflector_Technique : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Reflector_Technique() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Reflector_Technique(Engine * engine, const std::shared_ptr<std::vector<CameraBuffer::CamStruct*>> & cameras, ECSSystemList & auxilliarySystems)
		: m_engine(engine), m_cameras(cameras), Graphics_Technique(PRIMARY_LIGHTING) {
		// Auxilliary Systems
		m_frameData = std::make_shared<ReflectorData>();
		GLuint data[] = { 0,0,0,0 };
		m_indirectQuad.write(0, sizeof(GLuint) * 4, &data);
		m_indirectQuad6Faces.write(0, sizeof(GLuint) * 4, &data);
		auxilliarySystems.addSystem(new ReflectorScheduler_System(m_engine, m_frameData));
		auxilliarySystems.addSystem(new ReflectorVisibility_System(m_frameData, cameras));
		auxilliarySystems.addSystem(new ReflectorSync_System(m_frameData));

		// Asset Loading
		m_shaderLighting = Shared_Shader(m_engine, "Core\\Reflector\\IBL_Parallax");
		m_shaderStencil = Shared_Shader(m_engine, "Core\\Reflector\\Stencil");
		m_shaderCopy = Shared_Shader(m_engine, "Core\\Reflector\\2D_To_Cubemap");
		m_shaderConvolute = Shared_Shader(m_engine, "Core\\Reflector\\Cube_Convolution");
		m_shapeCube = Shared_Primitive(m_engine, "cube");
		m_shapeQuad = Shared_Primitive(m_engine, "quad");

		// Preferences
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_ENVMAP_SIZE, m_frameData->envmapSize.x);
		preferences.addCallback(PreferenceState::C_ENVMAP_SIZE, m_aliveIndicator, [&](const float &f) {
			m_frameData->envmapSize = glm::ivec2(std::max(1u, (unsigned int)f));
			m_frameData->envmapOutOfDate = true;
			m_viewport->resize(glm::ivec2(m_frameData->envmapSize));
		});
		// Environment Map
		m_frameData->envmapSize = glm::ivec2(std::max(1u, (unsigned int)m_frameData->envmapSize.x));
		m_viewport = std::make_shared<Viewport>(engine, glm::ivec2(0), m_frameData->envmapSize);

		// Asset-Finished Callbacks		
		m_shapeCube->addCallback(m_aliveIndicator, [&]() mutable {
			m_frameData->shapeVertexCount = m_shapeCube->getSize();
		});
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
			m_indirectQuad.write(0, sizeof(GLuint) * 4, quadData);
			const GLuint quad6Data[4] = { (GLuint)m_shapeQuad->getSize(), 6, 0, 0 };
			m_indirectQuad6Faces.write(0, sizeof(GLuint) * 4, quad6Data);
		});

		// Clear state on world-unloaded
		m_engine->getModule_World().addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded) {
				clear();
				m_frameData->envmapOutOfDate = false;
			}
			else if (state == World_Module::finishLoading || state == World_Module::updated)
				m_frameData->envmapOutOfDate = true;
		});
	}


	// Public Interface Implementations
	inline virtual void prepareForNextFrame(const float & deltaTime) override {
		m_frameData->lightBuffer.endWriting();
		for (auto & viewInfo : m_frameData->viewInfo)
			viewInfo.visLights.endWriting();
	}
	inline virtual void updateTechnique(const float & deltaTime) override {
		// Exit Early
		if (m_enabled && m_shapeQuad->existsYet() && m_shaderCopy->existsYet() && m_shaderConvolute->existsYet()) {

			//if (m_engine->getActionState().isAction(ActionState::FIRE2))
			//	m_frameData->envmapOutOfDate = true;

			// Render important environment maps
			updateReflectors(deltaTime);
		}
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const CameraBuffer::CamStruct * camera) override {
		// Exit Early
		if (m_enabled && m_shapeCube->existsYet() && m_shaderLighting->existsYet() && m_shaderStencil->existsYet()) {
			size_t visibilityIndex = 0;
			bool found = false;
			for (size_t x = 0; x < m_cameras->size(); ++x)
				if (m_cameras->at(x) == camera) {
					visibilityIndex = x;
					found = true;
					break;
				}
			if (found)
				renderReflectors(deltaTime, viewport, visibilityIndex);
		}
	}


private:
	// Private Methods
	/** Render all the geometry for each reflector.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void updateReflectors(const float & deltaTime) {
		if (m_frameData->reflectorsToUpdate.size()) {
			m_viewCameras.resize(m_frameData->reflectorsToUpdate.size() * 6);
			for (auto & camera : m_viewCameras)
				if (!camera)
					camera = std::make_shared<CameraBuffer>();
			int index = 0;
			for (auto &[time, reflector, cameras] : m_frameData->reflectorsToUpdate) {
				for (int x = 0; x < 6; ++x) {
					// Set view-specific camera data
					m_viewCameras[index]->beginWriting();
					m_viewCameras[index]->replace(*cameras[x]);
					m_viewCameras[index]->pushChanges();

					// Render Graphics Pipeline
					m_viewCameras[index]->bind(2);
					constexpr const unsigned int flags = Graphics_Technique::GEOMETRY | Graphics_Technique::PRIMARY_LIGHTING | Graphics_Technique::SECONDARY_LIGHTING;
					m_engine->getModule_Graphics().renderScene(deltaTime, m_viewport, cameras[x], flags);
					m_viewCameras[index]->endWriting();

					// Copy lighting frame into cube-face
					m_viewport->m_gfxFBOS->bindForReading("LIGHTING", 0);
					m_frameData->envmapFBO.bindForWriting();
					m_shaderCopy->bind();
					m_shaderCopy->setUniform(0, x + reflector->m_cubeSpot);
					m_indirectQuad.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
					glBindVertexArray(m_shapeQuad->m_vaoID);
					glDrawArraysIndirect(GL_TRIANGLES, 0);
				}

				// Once cubemap is generated, convolute it
				m_shaderConvolute->bind();
				m_shaderConvolute->setUniform(0, reflector->m_cubeSpot);
				m_frameData->envmapFBO.bindForReading();
				m_indirectQuad6Faces.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
				for (unsigned int r = 1; r < 6; ++r) {
					// Ensure we are writing to MIP level r
					const unsigned int write_size = (unsigned int)std::max(1.0f, (floor((float)m_frameData->envmapSize.x / pow(2.0f, (float)r))));
					glViewport(0, 0, write_size, write_size);
					m_shaderConvolute->setUniform(1, (float)r / 5.0f);
					glNamedFramebufferTexture(m_frameData->envmapFBO.m_fboID, GL_COLOR_ATTACHMENT0, m_frameData->envmapFBO.m_textureID, r);

					// Ensure we are reading from MIP level r - 1
					glTextureParameteri(m_frameData->envmapFBO.m_textureID, GL_TEXTURE_BASE_LEVEL, r - 1);
					glTextureParameteri(m_frameData->envmapFBO.m_textureID, GL_TEXTURE_MAX_LEVEL, r - 1);

					// Convolute the 6 faces for this roughness level (RENDERS 6 TIMES)
					glDrawArraysIndirect(GL_TRIANGLES, 0);
				}

				// Reset texture, so it can be used for other component reflections
				glTextureParameteri(m_frameData->envmapFBO.m_textureID, GL_TEXTURE_BASE_LEVEL, 0);
				glTextureParameteri(m_frameData->envmapFBO.m_textureID, GL_TEXTURE_MAX_LEVEL, 5);
				glNamedFramebufferTexture(m_frameData->envmapFBO.m_fboID, GL_COLOR_ATTACHMENT0, m_frameData->envmapFBO.m_textureID, 0);
				reflector->m_updateTime = m_engine->getTime();
				Shader::Release();
				reflector->m_sceneOutOfDate = false;
				index++;
			}

			m_frameData->reflectorsToUpdate.clear();
			m_frameData->envmapOutOfDate = false;
		}
	}
	/** Render all the lights
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderReflectors(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const size_t & visibilityIndex) {
		if (m_frameData->viewInfo.size() && m_frameData->viewInfo[visibilityIndex].visLightCount) {
			glEnable(GL_STENCIL_TEST);
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);

			// Draw only into depth-stencil buffer
			m_shaderStencil->bind();										// Shader (reflector)
			viewport->m_gfxFBOS->bindForWriting("REFLECTION");				// Ensure writing to reflection FBO
			viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);				// Read from Geometry FBO
			glBindTextureUnit(4, m_frameData->envmapFBO.m_textureID);					// Reflection map (environment texture)
			m_frameData->viewInfo[visibilityIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);		// SSBO visible light indices
			m_frameData->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);	// Reflection buffer
			m_frameData->viewInfo[visibilityIndex].indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);				// Draw call buffer
			glBindVertexArray(m_shapeCube->m_vaoID);						// Quad VAO
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
	}
	/** Clear out the reflectors queued up for rendering. */
	inline void clear() {
		m_frameData->viewInfo.clear();
		m_frameData->reflectorsToUpdate.clear();
		m_frameData->lightBuffer.clear();
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	Shared_Primitive m_shapeCube, m_shapeQuad;
	Shared_Shader m_shaderLighting, m_shaderStencil, m_shaderCopy, m_shaderConvolute;
	StaticBuffer m_indirectQuad = StaticBuffer(sizeof(GLuint) * 4), m_indirectQuad6Faces = StaticBuffer(sizeof(GLuint) * 4);
	std::shared_ptr<Viewport> m_viewport;
	std::vector<std::shared_ptr<CameraBuffer>> m_viewCameras;


	// Shared Attributes
	std::shared_ptr<ReflectorData> m_frameData;
	std::shared_ptr<std::vector<CameraBuffer::CamStruct*>> m_cameras;
};

#endif // REFLECTOR_TECHNIQUE_H