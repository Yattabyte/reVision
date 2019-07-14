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
#include "Utilities/GL/StaticTripleBuffer.h"
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
	inline Reflector_Technique(Engine * engine, const std::shared_ptr<std::vector<Camera*>> & cameras, ECSSystemList & auxilliarySystems)
		: m_engine(engine), m_sceneCameras(cameras), Graphics_Technique(PRIMARY_LIGHTING) {
		// Auxilliary Systems
		m_frameData = std::make_shared<ReflectorData>();
		auxilliarySystems.addSystem(new ReflectorScheduler_System(m_engine, m_frameData));
		auxilliarySystems.addSystem(new ReflectorVisibility_System(m_frameData));
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
			m_viewport->resize(glm::ivec2(m_frameData->envmapSize), m_frameData->reflectorLayers);
		});
		// Environment Map
		m_frameData->envmapSize = glm::ivec2(std::max(1u, (unsigned int)m_frameData->envmapSize.x));
		m_viewport = std::make_shared<Viewport>(glm::ivec2(0), m_frameData->envmapSize);

		// Asset-Finished Callbacks		
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			// count, primCount, first, reserved
			const GLuint quadData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 };
			m_indirectQuad = StaticTripleBuffer(sizeof(GLuint) * 4, quadData);
			m_indirectQuadConvolute = StaticTripleBuffer(sizeof(GLuint) * 4, quadData);
		});

		// Clear state on world-unloaded
		m_engine->getModule_World().addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded) 
				clear();			
		});
	}


	// Public Interface Implementations
	inline virtual void prepareForNextFrame(const float & deltaTime) override {
		m_frameData->lightBuffer.endWriting();
		for (auto & drawBuffer : m_drawBuffers) {
			drawBuffer.bufferCamIndex.endWriting();
			drawBuffer.visLights.endWriting();
			drawBuffer.indirectShape.endWriting();
		}
		m_indirectQuad.endWriting();
		m_indirectQuadConvolute.endWriting();
		m_drawIndex = 0;
	}
	inline virtual void updateTechnique(const float & deltaTime) override {
		// Link together the dimensions of view info to that of the viewport vectors
		m_frameData->viewInfo.resize(m_sceneCameras->size());

		// Exit Early
		if (m_enabled && m_shapeQuad->existsYet() && m_shaderCopy->existsYet() && m_shaderConvolute->existsYet())
			updateReflectors(deltaTime);		
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives) override {
		// Exit Early
		if (m_enabled && m_frameData->viewInfo.size() && m_shapeCube->existsYet() && m_shaderLighting->existsYet() && m_shaderStencil->existsYet()) {
			if (m_drawIndex >= m_drawBuffers.size())
				m_drawBuffers.resize(m_drawIndex + 1);
			auto & drawBuffer = m_drawBuffers[m_drawIndex];
			auto &camBufferIndex = drawBuffer.bufferCamIndex;
			auto &lightBufferIndex = drawBuffer.visLights;
			camBufferIndex.beginWriting();
			lightBufferIndex.beginWriting();
			drawBuffer.indirectShape.beginWriting();

			// Accumulate all visibility info for the cameras passed in
			std::vector<glm::ivec2> camIndices;
			std::vector<GLint> lightIndices;
			for (auto &[camIndex, layer] : perspectives) {
				const std::vector<glm::ivec2> tempIndices(m_frameData->viewInfo[camIndex].lightIndices.size(), { camIndex, layer });
				camIndices.insert(camIndices.end(), tempIndices.begin(), tempIndices.end());
				lightIndices.insert(lightIndices.end(), m_frameData->viewInfo[camIndex].lightIndices.begin(), m_frameData->viewInfo[camIndex].lightIndices.end());
			}

			if (lightIndices.size()) {
				// Write accumulated data
				camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
				lightBufferIndex.write(0, sizeof(GLuint) * lightIndices.size(), lightIndices.data());
				const GLuint dataShape[] = { (GLuint)m_shapeCube->getSize(), (GLuint)lightIndices.size(), 0,0 };
				drawBuffer.indirectShape.write(0, sizeof(GLuint) * 4, &dataShape);

				// Render lights
				renderReflectors(deltaTime, viewport);

				m_drawIndex++;
			}
		}
	}


private:
	// Private Methods
	/** Render all the geometry for each reflector.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void updateReflectors(const float & deltaTime) {
		auto clientTime = m_engine->getTime();
		if (m_frameData->reflectorsToUpdate.size()) {
			m_viewport->resize(m_frameData->envmapSize, m_frameData->reflectorLayers);
			m_indirectQuad.beginWriting();
			m_indirectQuadConvolute.beginWriting();

			// Accumulate Perspective Data
			std::vector<std::pair<int, int>> perspectives;
			perspectives.reserve(m_frameData->reflectorsToUpdate.size());
			for (auto &[importance, time, reflectorSpot, camera] : m_frameData->reflectorsToUpdate) {
				// Accumulate all visibility info for the cameras passed in
				int visibilityIndex = 0;
				bool found = false;
				for (int y = 0; y < m_sceneCameras->size(); ++y)
					if (m_sceneCameras->at(y) == camera) {
						visibilityIndex = y;
						found = true;
						break;
					}
				if (found) 
					perspectives.push_back({ visibilityIndex, reflectorSpot });				
				*time = clientTime;
				camera->setEnabled(false);
			}
			GLuint instanceCount = perspectives.size(), convoluteCount = m_frameData->reflectorLayers;
			m_indirectQuad.write(sizeof(GLuint), sizeof(GLuint), &instanceCount);
			m_indirectQuadConvolute.write(sizeof(GLuint), sizeof(GLuint), &convoluteCount);

			// Update all reflectors at once
			m_engine->getModule_Graphics().renderScene(deltaTime, m_viewport, perspectives, Graphics_Technique::GEOMETRY | Graphics_Technique::PRIMARY_LIGHTING | Graphics_Technique::SECONDARY_LIGHTING);

			// Copy all lighting results into cube faces, generating cubemaps
			m_viewport->m_gfxFBOS->bindForReading("LIGHTING", 0);
			m_frameData->envmapFBO.bindForWriting(0);
			m_shaderCopy->bind();
			m_indirectQuad.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			glDisable(GL_STENCIL_TEST);
			glBindVertexArray(m_shapeQuad->m_vaoID);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// Convolute all completed cubemaps, not just what was done this frame
			m_shaderConvolute->bind();
			m_indirectQuadConvolute.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			m_frameData->envmapFBO.bindForReading(0);
			for (unsigned int r = 1; r < 6; ++r) {
				// Ensure we are writing to MIP level r
				const unsigned int write_size = (unsigned int)std::max(1.0f, (floor((float)m_frameData->envmapSize.x / pow(2.0f, (float)r))));
				glViewport(0, 0, write_size, write_size);
				m_shaderConvolute->setUniform(1, (float)r / 5.0f);
				m_frameData->envmapFBO.bindForWriting(r);

				// Convolute the 6 faces for this roughness level
				glDrawArraysIndirect(GL_TRIANGLES, 0);
			}

			m_frameData->reflectorsToUpdate.clear();			
		}
	}
	/** Render all the lights
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderReflectors(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) {
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		// Draw only into depth-stencil buffer
		m_shaderStencil->bind();										// Shader (reflector)
		viewport->m_gfxFBOS->bindForWriting("REFLECTION");				// Ensure writing to reflection FBO
		viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);				// Read from Geometry FBO
		glBindTextureUnit(4, m_frameData->envmapFBO.m_textureID);					// Reflection map (environment texture)
		m_drawBuffers[m_drawIndex].bufferCamIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_drawBuffers[m_drawIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);	// SSBO visible light indices
		m_frameData->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);	// Reflection buffer
		m_drawBuffers[m_drawIndex].indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);				// Draw call buffer
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
	/** Clear out the reflectors queued up for rendering. */
	inline void clear() {
		m_frameData->viewInfo.clear();
		m_frameData->reflectorsToUpdate.clear();
		m_drawBuffers.clear();
		m_drawIndex = 0;
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	Shared_Primitive m_shapeCube, m_shapeQuad;
	Shared_Shader m_shaderLighting, m_shaderStencil, m_shaderCopy, m_shaderConvolute;
	StaticTripleBuffer m_indirectQuad = StaticTripleBuffer(sizeof(GLuint) * 4), m_indirectQuadConvolute = StaticTripleBuffer(sizeof(GLuint) * 4);
	std::shared_ptr<Viewport> m_viewport;
	struct DrawBuffers {
		DynamicBuffer bufferCamIndex;
		DynamicBuffer visLights;
		StaticTripleBuffer indirectShape = StaticTripleBuffer(sizeof(GLuint) * 4);
	};
	int m_drawIndex = 0;
	std::vector<DrawBuffers> m_drawBuffers;


	// Shared Attributes
	std::shared_ptr<ReflectorData> m_frameData;
	std::shared_ptr<std::vector<Camera*>> m_sceneCameras;
};

#endif // REFLECTOR_TECHNIQUE_H