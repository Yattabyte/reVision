#pragma once
#ifndef PROP_TECHNIQUE_H
#define PROP_TECHNIQUE_H

#include "Modules/Graphics/Geometry/Geometry_Technique.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Modules/Graphics/Geometry/Prop/PropData.h"
#include "Modules/Graphics/Geometry/Prop/PropUpload_System.h"
#include "Modules/Graphics/Geometry/Prop/PropVisibility_System.h"
#include "Modules/Graphics/Geometry/Prop/PropSync_System.h"
#include "Modules/World/ECS/components.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Engine.h"


/** A core rendering technique for rendering props from a given viewing perspective. */
class Prop_Technique : public Geometry_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Prop_Technique() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Prop_Technique(Engine * engine, const std::shared_ptr<std::vector<Camera*>> & viewports, ECSSystemList & auxilliarySystems)
		: m_engine(engine), m_cameras(viewports) {
		// Auxilliary Systems
		m_frameData = std::make_shared<PropData>();
		auxilliarySystems.addSystem(new PropUpload_System(engine, m_frameData));
		auxilliarySystems.addSystem(new PropVisibility_System(m_frameData, viewports));
		auxilliarySystems.addSystem(new PropSync_System(m_frameData));

		// Asset Loading
		m_shaderCull = Shared_Shader(m_engine, "Core\\Props\\culling");
		m_shaderGeometry = Shared_Shader(m_engine, "Core\\Props\\geometry");
		m_shaderShadowCull = Shared_Shader(m_engine, "Core\\Props\\shadow culling");
		m_shaderShadowGeometry = Shared_Shader(m_engine, "Core\\Props\\shadow");
		m_shapeCube = Shared_Primitive(m_engine, "cube");
		
		// Clear state on world-unloaded
		m_engine->getModule_World().addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded)
				clear();
		});
	}


	// Public Interface Implementations
	inline virtual void prepareForNextFrame(const float & deltaTime) override {
		m_frameData->modelBuffer.endWriting();
		m_frameData->skeletonBuffer.endWriting();
		for (auto & drawBuffer : m_drawData) {
			drawBuffer.bufferCamIndex.endWriting();
			drawBuffer.bufferPropIndex.endWriting();
			drawBuffer.bufferCulling.endWriting();
			drawBuffer.bufferRender.endWriting();
			drawBuffer.bufferSkeletonIndex.endWriting();
		}
		m_drawIndex = 0;
	}
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives) override {
		// Exit Early
		if (m_enabled && m_frameData->viewInfo.size() && m_shapeCube->existsYet() && m_shaderCull->existsYet() && m_shaderGeometry->existsYet()) {
			if (m_drawIndex >= m_drawData.size())
				m_drawData.resize(m_drawIndex + 1);
			auto & drawBuffer = m_drawData[m_drawIndex];
			auto &camBufferIndex = drawBuffer.bufferCamIndex;
			auto &propIndexBuffer = drawBuffer.bufferPropIndex;
			auto &propCullingBuffer = drawBuffer.bufferCulling;
			auto &propRenderBuffer = drawBuffer.bufferRender;
			auto &propSkeletonBuffer = drawBuffer.bufferSkeletonIndex;
			camBufferIndex.beginWriting();
			propIndexBuffer.beginWriting();
			propCullingBuffer.beginWriting();
			propRenderBuffer.beginWriting();
			propSkeletonBuffer.beginWriting();

			// Accumulate all visibility info for the cameras passed in
			std::vector<glm::ivec2> camIndices;
			std::vector<glm::ivec4> cullingDrawData, renderingDrawData;
			std::vector<GLuint> visibleIndices;
			std::vector<int> skeletonData;
			for (auto &[camIndex, layer] : perspectives) {
				const std::vector<glm::ivec2> tempIndices(m_frameData->viewInfo[camIndex].visibleIndices.size(), { camIndex, layer });
				camIndices.insert(camIndices.end(), tempIndices.begin(), tempIndices.end());
				visibleIndices.insert(visibleIndices.end(), m_frameData->viewInfo[camIndex].visibleIndices.begin(), m_frameData->viewInfo[camIndex].visibleIndices.end());
				cullingDrawData.insert(cullingDrawData.end(), m_frameData->viewInfo[camIndex].cullingDrawData.begin(), m_frameData->viewInfo[camIndex].cullingDrawData.end());
				renderingDrawData.insert(renderingDrawData.end(), m_frameData->viewInfo[camIndex].renderingDrawData.begin(), m_frameData->viewInfo[camIndex].renderingDrawData.end());
				skeletonData.insert(skeletonData.end(), m_frameData->viewInfo[camIndex].skeletonData.begin(), m_frameData->viewInfo[camIndex].skeletonData.end());
			}

			if (visibleIndices.size()) {
				// Write accumulated data
				camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
				propIndexBuffer.write(0, sizeof(GLuint) * visibleIndices.size(), visibleIndices.data());
				propCullingBuffer.write(0, sizeof(glm::ivec4) * cullingDrawData.size(), cullingDrawData.data());
				propRenderBuffer.write(0, sizeof(glm::ivec4) * renderingDrawData.size(), renderingDrawData.data());
				propSkeletonBuffer.write(0, sizeof(int) * skeletonData.size(), skeletonData.data());

				// Apply occlusion culling and render props
				camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
				m_frameData->modelBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
				propIndexBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 5);
				m_frameData->skeletonBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
				propSkeletonBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);

				// Draw bounding boxes for each model, filling render buffer on successful rasterization
				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
				glDepthFunc(GL_LEQUAL);
				glDepthMask(GL_FALSE);
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				m_shaderCull->bind();
				viewport->m_gfxFBOS->bindForWriting("GEOMETRY");
				glBindVertexArray(m_shapeCube->m_vaoID);
				propCullingBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
				propRenderBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
				glMultiDrawArraysIndirect(GL_TRIANGLES, 0, visibleIndices.size(), 0);
				glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, 0);

				// Draw geometry using the populated render buffer
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDepthMask(GL_TRUE);
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				viewport->m_gfxFBOS->clearDepthStencil();
				m_shaderGeometry->bind();
				glBindVertexArray(m_frameData->m_geometryVAOID);
				glBindTextureUnit(0, m_frameData->m_materialArrayID);
				propRenderBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
				glMultiDrawArraysIndirect(GL_TRIANGLES, 0, visibleIndices.size(), 0);
				m_drawIndex++;
			}			
		}
	}
	inline virtual void cullShadows(const float & deltaTime, const std::vector<std::pair<int, int>> & perspectives) override {
		// Exit Early
		if (m_enabled && m_frameData->viewInfo.size() && m_shapeCube->existsYet() && m_shaderShadowCull->existsYet() && m_shaderShadowGeometry->existsYet()) {
			if (m_drawIndex >= m_drawData.size())
				m_drawData.resize(m_drawIndex + 1);
			auto & drawBuffer = m_drawData[m_drawIndex];
			auto &camBufferIndex = drawBuffer.bufferCamIndex;
			auto &propIndexBuffer = drawBuffer.bufferPropIndex;
			auto &propCullingBuffer = drawBuffer.bufferCulling;
			auto &propRenderBuffer = drawBuffer.bufferRender;
			auto &propSkeletonBuffer = drawBuffer.bufferSkeletonIndex;
			camBufferIndex.beginWriting();
			propIndexBuffer.beginWriting();
			propCullingBuffer.beginWriting();
			propRenderBuffer.beginWriting();
			propSkeletonBuffer.beginWriting();
			std::vector<glm::ivec2> camIndices;
			std::vector<glm::ivec4> cullingDrawData, renderingDrawData;
			std::vector<GLuint> visibleIndices;
			std::vector<int> skeletonData;
			// Accumulate all visibility info for the cameras passed in
			for (auto &[camIndex, layer] : perspectives) {
				const std::vector<glm::ivec2> tempIndices(m_frameData->viewInfo[camIndex].visibleIndices.size(), { camIndex, layer });
				camIndices.insert(camIndices.end(), tempIndices.begin(), tempIndices.end());
				visibleIndices.insert(visibleIndices.end(), m_frameData->viewInfo[camIndex].visibleIndices.begin(), m_frameData->viewInfo[camIndex].visibleIndices.end());
				cullingDrawData.insert(cullingDrawData.end(), m_frameData->viewInfo[camIndex].cullingDrawData.begin(), m_frameData->viewInfo[camIndex].cullingDrawData.end());
				renderingDrawData.insert(renderingDrawData.end(), m_frameData->viewInfo[camIndex].renderingDrawData.begin(), m_frameData->viewInfo[camIndex].renderingDrawData.end());
				skeletonData.insert(skeletonData.end(), m_frameData->viewInfo[camIndex].skeletonData.begin(), m_frameData->viewInfo[camIndex].skeletonData.end());
			}
			// Write all visibility info to a set of buffers
			if (visibleIndices.size()) {
				camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
				propIndexBuffer.write(0, sizeof(GLuint) * visibleIndices.size(), visibleIndices.data());
				propCullingBuffer.write(0, sizeof(glm::ivec4) * cullingDrawData.size(), cullingDrawData.data());
				propRenderBuffer.write(0, sizeof(glm::ivec4) * renderingDrawData.size(), renderingDrawData.data());
				propSkeletonBuffer.write(0, sizeof(int) * skeletonData.size(), skeletonData.data());

				// Apply occlusion culling and render props
				camBufferIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
				m_frameData->modelBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
				propIndexBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 5);
				m_frameData->skeletonBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);
				propSkeletonBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);

				// Draw bounding boxes for each model, filling render buffer on successful rasterization
				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);
				glDepthFunc(GL_LEQUAL);
				glDepthMask(GL_FALSE);
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				m_shaderShadowCull->bind();
				glBindVertexArray(m_shapeCube->m_vaoID);
				propCullingBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
				propRenderBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
				glMultiDrawArraysIndirect(GL_TRIANGLES, 0, visibleIndices.size(), 0);
				glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, 0);
				m_count = visibleIndices.size();
			}
		}
	}
	inline virtual void renderShadows(const float & deltaTime) override {
		// Exit Early
		if (m_enabled && m_shapeCube->existsYet() && m_shaderShadowCull->existsYet() && m_shaderShadowGeometry->existsYet()) {					
			if (m_count) {
				// Draw geometry using the populated render buffer
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDepthMask(GL_TRUE);
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				glFrontFace(GL_CW);
				m_shaderShadowGeometry->bind();
				glBindVertexArray(m_frameData->m_geometryVAOID);
				glBindTextureUnit(0, m_frameData->m_materialArrayID);
				m_drawData[m_drawIndex].bufferRender.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
				glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_count, 0);
				glFrontFace(GL_CCW);
				glCullFace(GL_BACK);
				m_drawIndex++;
			}
		}	
	}


private:
	// Private Methods
	/** Clear out the props queued up for rendering. */
	inline void clear() {
		m_frameData->viewInfo.clear();
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Shader m_shaderCull, m_shaderGeometry, m_shaderShadowCull, m_shaderShadowGeometry;
	Shared_Primitive m_shapeCube;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	struct DrawData {
		DynamicBuffer bufferCamIndex, bufferPropIndex, bufferCulling, bufferRender, bufferSkeletonIndex;
	};
	int m_drawIndex = 0;
	size_t m_count = 0ull;
	std::vector<DrawData> m_drawData;


	// Shared Attributes
	std::shared_ptr<PropData> m_frameData;
	std::shared_ptr<std::vector<Camera*>> m_cameras;
};

#endif // PROP_TECHNIQUE_H