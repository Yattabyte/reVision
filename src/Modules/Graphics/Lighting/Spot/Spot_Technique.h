#pragma once
#ifndef SPOT_TECHNIQUE_H
#define SPOT_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/Spot/SpotData.h"
#include "Modules/Graphics/Lighting/Spot/SpotVisibility_System.h"
#include "Modules/Graphics/Lighting/Spot/SpotSync_System.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/StaticTripleBuffer.h"
#include "Engine.h"


/** A core lighting technique responsible for all spot lights. */
class Spot_Technique : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Spot_Technique() {
		// Update indicator
		*m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Spot_Technique(Engine * engine, const std::shared_ptr<ShadowData> & shadowData, const std::shared_ptr<std::vector<Camera*>> & cameras, ecsSystemList & auxilliarySystems)
		: m_engine(engine), m_cameras(cameras), Graphics_Technique(PRIMARY_LIGHTING) {
		// Auxilliary Systems
		m_frameData = std::make_shared<SpotData>();
		m_frameData->shadowData = shadowData;
		auxilliarySystems.makeSystem<SpotVisibility_System>(m_frameData);
		auxilliarySystems.makeSystem<SpotSync_System>(m_frameData);

		// Asset Loading
		m_shader_Lighting = Shared_Shader(engine, "Core\\Spot\\Light");
		m_shader_Stencil = Shared_Shader(engine, "Core\\Spot\\Stencil");
		m_shapeCone = Shared_Auto_Model(engine, "cone");

		// Clear state on world-unloaded
		m_engine->getModule_World().addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded)
				clear();
		});
	}


	// Public Interface Implementations
	inline virtual void prepareForNextFrame(const float & deltaTime) override {
		m_frameData->lightBuffer.endWriting();
		for (auto & drawBuffer : m_drawData) {
			drawBuffer.bufferCamIndex.endWriting();
			drawBuffer.visLights.endWriting();
			drawBuffer.indirectShape.endWriting();
		}
		m_drawIndex = 0;
	}
	inline virtual void updateTechnique(const float & deltaTime) override {
		// Link together the dimensions of view info to that of the viewport vectors
		m_frameData->viewInfo.resize(m_cameras->size());
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives) override {
		// Render direct lights	
		if (m_enabled && m_frameData->viewInfo.size() && m_shapeCone->existsYet() && m_shader_Lighting->existsYet() && m_shader_Stencil->existsYet()) {
			if (m_drawIndex >= m_drawData.size())
				m_drawData.resize(size_t(m_drawIndex) + 1ull);
			auto & drawBuffer = m_drawData[m_drawIndex];
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
				const GLuint dataShape[] = { (GLuint)m_shapeCone->getSize(), (GLuint)lightIndices.size(), 0,0 };
				drawBuffer.indirectShape.write(0, sizeof(GLuint) * 4, &dataShape);

				// Render lights
				renderLights(deltaTime, viewport);

				m_drawIndex++;
			}
		}
	}


private:
	// Private Methods
	/** Render all the lights.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	inline void renderLights(const float & deltaTime, const std::shared_ptr<Viewport> & viewport) {
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
		glStencilMask(0xFF);

		// Draw only into depth-stencil buffer
		m_shader_Stencil->bind();																		// Shader (spot)
		viewport->m_gfxFBOS->bindForWriting("LIGHTING");												// Ensure writing to lighting FBO
		viewport->m_gfxFBOS->bindForReading("GEOMETRY", 0);												// Read from Geometry FBO
		glBindTextureUnit(4, m_frameData->shadowData->shadowFBO.m_texDepth);							// Shadow map (depth texture)		
		m_drawData[m_drawIndex].bufferCamIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);	
		m_drawData[m_drawIndex].visLights.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);	// SSBO visible light indices
		m_frameData->lightBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
		m_drawData[m_drawIndex].indirectShape.bindBuffer(GL_DRAW_INDIRECT_BUFFER);		// Draw call buffer
		glBindVertexArray(m_shapeCone->m_vaoID);
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0, 0);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Now draw into color buffers
		m_shader_Lighting->bind();
		m_shader_Lighting->setUniform(0, m_frameData->shadowData->shadowSizeRCP);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		glClear(GL_STENCIL_BUFFER_BIT);
		glCullFace(GL_BACK);
		glDepthMask(GL_TRUE);
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
	}
	/** Clear out the lights and shadows queued up for rendering. */
	inline void clear() {
		m_frameData->viewInfo.clear();
		m_drawData.clear();
		m_drawIndex = 0;
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	Shared_Shader m_shader_Lighting, m_shader_Stencil;
	Shared_Auto_Model m_shapeCone;
	struct DrawData {
		DynamicBuffer bufferCamIndex;
		DynamicBuffer visLights;
		StaticTripleBuffer indirectShape = StaticTripleBuffer(sizeof(GLuint) * 4);
	};
	int m_drawIndex = 0;
	std::vector<DrawData> m_drawData;


	// Shared Attributes
	std::shared_ptr<SpotData> m_frameData;
	std::shared_ptr<std::vector<Camera*>> m_cameras;
};

#endif // SPOT_TECHNIQUE_H