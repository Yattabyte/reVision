#pragma once
#ifndef PROP_TECHNIQUE_H
#define PROP_TECHNIQUE_H

#include "Modules/Graphics/Geometry/Geometry_Technique.h"
#include "Modules/Graphics/Geometry/Prop/PropData.h"
#include "Modules/Graphics/Geometry/Prop/PropUpload_System.h"
#include "Modules/Graphics/Geometry/Prop/PropVisibility_System.h"
#include "Modules/Graphics/Geometry/Prop/PropSync_System.h"
#include "Modules/ECS/ecsSystem.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Engine.h"


/** A core rendering technique for rendering props from a given viewing perspective. */
class Prop_Technique final : public Geometry_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Prop_Technique() = default;
	/** Constructor. */
	inline Prop_Technique(Engine* engine, const std::shared_ptr<std::vector<Camera*>>& viewports)
		: m_engine(engine), m_cameras(viewports) {
		// Auxiliary Systems
		m_frameData = std::make_shared<PropData>();
		m_auxilliarySystems.makeSystem<PropUpload_System>(engine, m_frameData);
		m_auxilliarySystems.makeSystem<PropVisibility_System>(m_frameData, viewports);
		m_auxilliarySystems.makeSystem<PropSync_System>(m_frameData);

		// Asset Loading
		m_shaderCull = Shared_Shader(engine, "Core\\Props\\culling");
		m_shaderGeometry = Shared_Shader(engine, "Core\\Props\\geometry");
		m_shaderShadowCull = Shared_Shader(engine, "Core\\Props\\shadow culling");
		m_shaderShadowGeometry = Shared_Shader(engine, "Core\\Props\\shadow");
		m_shapeCube = Shared_Auto_Model(engine, "cube");
	}


	// Public Interface Implementations
	inline virtual void prepareForNextFrame(const float& deltaTime) override final {
		m_frameData->modelBuffer.endReading();
		m_frameData->skeletonBuffer.endReading();
		m_drawIndex = 0;
		clear();
	}
	inline virtual void updateTechnique(const float& deltaTime, ecsWorld& world) override final {
		// Link together the dimensions of view info to that of the viewport vectors
		m_frameData->viewInfo.resize(m_cameras->size());
		world.updateSystems(m_auxilliarySystems, deltaTime);
	}
	inline virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::shared_ptr<RH_Volume>& rhVolume, const std::vector<std::pair<int, int>>& perspectives) override final {
		// Exit Early
		if (m_enabled && m_frameData->viewInfo.size() && m_shapeCube->existsYet() && m_shaderCull->existsYet() && m_shaderGeometry->existsYet()) {
			if (m_drawIndex >= m_drawData.size())
				m_drawData.resize(size_t(m_drawIndex) + 1ull);
			auto& drawBuffer = m_drawData[m_drawIndex];
			auto& camBufferIndex = drawBuffer.bufferCamIndex;
			auto& propIndexBuffer = drawBuffer.bufferPropIndex;
			auto& propCullingBuffer = drawBuffer.bufferCulling;
			auto& propRenderBuffer = drawBuffer.bufferRender;
			auto& propSkeletonBuffer = drawBuffer.bufferSkeletonIndex;

			// Accumulate all visibility info for the cameras passed in
			std::vector<glm::ivec2> camIndices;
			std::vector<glm::ivec4> cullingDrawData, renderingDrawData;
			std::vector<GLuint> visibleIndices;
			std::vector<int> skeletonData;
			for (auto& [camIndex, layer] : perspectives) {
				const std::vector<glm::ivec2> tempIndices(m_frameData->viewInfo[camIndex].visibleIndices.size(), { camIndex, layer });
				camIndices.insert(camIndices.end(), tempIndices.begin(), tempIndices.end());
				visibleIndices.insert(visibleIndices.end(), m_frameData->viewInfo[camIndex].visibleIndices.begin(), m_frameData->viewInfo[camIndex].visibleIndices.end());
				cullingDrawData.insert(cullingDrawData.end(), m_frameData->viewInfo[camIndex].cullingDrawData.begin(), m_frameData->viewInfo[camIndex].cullingDrawData.end());
				renderingDrawData.insert(renderingDrawData.end(), m_frameData->viewInfo[camIndex].renderingDrawData.begin(), m_frameData->viewInfo[camIndex].renderingDrawData.end());
				skeletonData.insert(skeletonData.end(), m_frameData->viewInfo[camIndex].skeletonData.begin(), m_frameData->viewInfo[camIndex].skeletonData.end());
			}

			// Write all visibility info to a set of buffers
			if (visibleIndices.size()) {
				// Prepare for writing
				camBufferIndex.beginWriting();
				propIndexBuffer.beginWriting();
				propCullingBuffer.beginWriting();
				propRenderBuffer.beginWriting();
				propSkeletonBuffer.beginWriting();

				// Write accumulated data
				camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
				propIndexBuffer.write(0, sizeof(GLuint) * visibleIndices.size(), visibleIndices.data());
				propCullingBuffer.write(0, sizeof(glm::ivec4) * cullingDrawData.size(), cullingDrawData.data());
				propRenderBuffer.write(0, sizeof(glm::ivec4) * renderingDrawData.size(), renderingDrawData.data());
				propSkeletonBuffer.write(0, sizeof(int) * skeletonData.size(), skeletonData.data());

				// End writing
				camBufferIndex.endWriting();
				propIndexBuffer.endWriting();
				propCullingBuffer.endWriting();
				propRenderBuffer.endWriting();
				propSkeletonBuffer.endWriting();

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
				viewport->m_gfxFBOS->bindForWriting("DEPTH-ONLY"); // use previous frame's depth
				glBindVertexArray(m_shapeCube->m_vaoID);
				propCullingBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
				propRenderBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
				glMultiDrawArraysIndirect(GL_TRIANGLES, 0, (GLsizei)visibleIndices.size(), 0);
				glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, 0);

				// Draw geometry using the populated render buffer
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glDepthMask(GL_TRUE);
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				viewport->m_gfxFBOS->bindForWriting("GEOMETRY"); // fill current frame's depth
				m_shaderGeometry->bind();
				glBindVertexArray(m_frameData->m_geometryVAOID);
				glBindTextureUnit(0, m_frameData->m_materialArrayID);
				propRenderBuffer.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
				glMultiDrawArraysIndirect(GL_TRIANGLES, 0, (GLsizei)visibleIndices.size(), 0);

				// Copy depth for next frame
				viewport->m_gfxFBOS->bindForWriting("DEPTH-ONLY");
				const auto& [sourceID, destinationID] = std::make_pair(viewport->m_gfxFBOS->getFboID("GEOMETRY"), viewport->m_gfxFBOS->getFboID("DEPTH-ONLY"));
				const auto& size = viewport->m_dimensions;
				glBlitNamedFramebuffer(sourceID, destinationID, 0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

				camBufferIndex.endReading();
				propIndexBuffer.endReading();
				propCullingBuffer.endReading();
				propRenderBuffer.endReading();
				propSkeletonBuffer.endReading();
				Shader::Release();
				m_drawIndex++;
			}
		}
	}
	inline virtual void cullShadows(const float& deltaTime, const std::vector<std::pair<int, int>>& perspectives) override final {
		// Exit Early
		if (m_enabled && m_frameData->viewInfo.size() && m_shapeCube->existsYet() && m_shaderShadowCull->existsYet() && m_shaderShadowGeometry->existsYet()) {
			if (m_drawIndex >= m_drawData.size())
				m_drawData.resize(size_t(m_drawIndex) + 1ull);
			auto& drawBuffer = m_drawData[m_drawIndex];
			auto& camBufferIndex = drawBuffer.bufferCamIndex;
			auto& propIndexBuffer = drawBuffer.bufferPropIndex;
			auto& propCullingBuffer = drawBuffer.bufferCulling;
			auto& propRenderBuffer = drawBuffer.bufferRender;
			auto& propSkeletonBuffer = drawBuffer.bufferSkeletonIndex;

			// Accumulate all visibility info for the cameras passed in
			std::vector<glm::ivec2> camIndices;
			std::vector<glm::ivec4> cullingDrawData, renderingDrawData;
			std::vector<GLuint> visibleIndices;
			std::vector<int> skeletonData;
			for (auto& [camIndex, layer] : perspectives) {
				const std::vector<glm::ivec2> tempIndices(m_frameData->viewInfo[camIndex].visibleIndices.size(), { camIndex, layer });
				camIndices.insert(camIndices.end(), tempIndices.begin(), tempIndices.end());
				visibleIndices.insert(visibleIndices.end(), m_frameData->viewInfo[camIndex].visibleIndices.begin(), m_frameData->viewInfo[camIndex].visibleIndices.end());
				cullingDrawData.insert(cullingDrawData.end(), m_frameData->viewInfo[camIndex].cullingDrawData.begin(), m_frameData->viewInfo[camIndex].cullingDrawData.end());
				renderingDrawData.insert(renderingDrawData.end(), m_frameData->viewInfo[camIndex].renderingDrawData.begin(), m_frameData->viewInfo[camIndex].renderingDrawData.end());
				skeletonData.insert(skeletonData.end(), m_frameData->viewInfo[camIndex].skeletonData.begin(), m_frameData->viewInfo[camIndex].skeletonData.end());
			}

			// Write all visibility info to a set of buffers
			if (visibleIndices.size()) {
				// Prepare for writing
				camBufferIndex.beginWriting();
				propIndexBuffer.beginWriting();
				propCullingBuffer.beginWriting();
				propRenderBuffer.beginWriting();
				propSkeletonBuffer.beginWriting();

				// Write accumulated data
				camBufferIndex.write(0, sizeof(glm::ivec2) * camIndices.size(), camIndices.data());
				propIndexBuffer.write(0, sizeof(GLuint) * visibleIndices.size(), visibleIndices.data());
				propCullingBuffer.write(0, sizeof(glm::ivec4) * cullingDrawData.size(), cullingDrawData.data());
				propRenderBuffer.write(0, sizeof(glm::ivec4) * renderingDrawData.size(), renderingDrawData.data());
				propSkeletonBuffer.write(0, sizeof(int) * skeletonData.size(), skeletonData.data());

				// End writing
				camBufferIndex.endWriting();
				propIndexBuffer.endWriting();
				propCullingBuffer.endWriting();
				propRenderBuffer.endWriting();
				propSkeletonBuffer.endWriting();

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
				glMultiDrawArraysIndirect(GL_TRIANGLES, 0, (GLsizei)visibleIndices.size(), 0);
				glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, 0);
				m_count = visibleIndices.size();

				Shader::Release();
			}
		}
	}
	inline virtual void renderShadows(const float& deltaTime) override final {
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
				glMultiDrawArraysIndirect(GL_TRIANGLES, 0, (GLsizei)m_count, 0);
				glFrontFace(GL_CCW);
				glCullFace(GL_BACK);
				auto& drawBuffer = m_drawData[m_drawIndex];
				drawBuffer.bufferCamIndex.endReading();
				drawBuffer.bufferPropIndex.endReading();
				drawBuffer.bufferCulling.endReading();
				drawBuffer.bufferRender.endReading();
				drawBuffer.bufferSkeletonIndex.endReading();
				Shader::Release();
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
	Engine* m_engine = nullptr;
	Shared_Shader m_shaderCull, m_shaderGeometry, m_shaderShadowCull, m_shaderShadowGeometry;
	Shared_Auto_Model m_shapeCube;
	struct DrawData {
		DynamicBuffer<> bufferCamIndex, bufferPropIndex, bufferCulling, bufferRender, bufferSkeletonIndex;
	};
	int m_drawIndex = 0;
	size_t m_count = 0ull;
	std::vector<DrawData> m_drawData;
	ecsSystemList m_auxilliarySystems;


	// Shared Attributes
	std::shared_ptr<PropData> m_frameData;
	std::shared_ptr<std::vector<Camera*>> m_cameras;
};

#endif // PROP_TECHNIQUE_H