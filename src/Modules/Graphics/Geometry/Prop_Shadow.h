#pragma once
#ifndef PROP_SHADOW_H
#define PROP_SHADOW_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Modules/Graphics/Geometry/Prop_Renderer.h"
#include "Assets/Shader.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Engine.h"
#include <vector>


/** A reusable secondary technique for rendering props into shadow maps. */
class Prop_Shadow {
public:
	// Public Enumerations
	const enum RenderType_Flags {
		RenderStatic = 0b0000'0001,
		RenderDynamic = 0b0000'0010,
		RenderAll = 0b0000'0011,
	};


	// Public (de)Constructors
	/** Destructor. */
	inline ~Prop_Shadow() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Prop_Shadow(Engine * engine, const unsigned int & instanceCount, const unsigned int & flags, const Shared_Shader & shaderCull, const Shared_Shader & shaderShadow, Prop_View * propView)
		: m_engine(engine), m_instanceCount(instanceCount), m_flags(flags), m_shaderCull(shaderCull), m_shaderShadow(shaderShadow), m_propView(propView) {
		// Asset Loading
		m_shapeCube = Shared_Primitive(m_engine, "cube");
		m_modelsVAO = &m_engine->getManager_Models().getVAO();

		// World-Changed Callback
		m_engine->getModule_World().addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded)
				clear();
		});
	}


	// Public Interface Implementations
	inline void renderShadows(const float & deltaTime) {
		// Exit Early
		if (!m_shapeCube->existsYet() || !m_shaderCull->existsYet() || !m_shaderShadow->existsYet())
			return;

		// Populate render-lists
		m_bufferPropIndex.beginWriting();
		m_bufferCulling.beginWriting();
		m_bufferRender.beginWriting();
		m_bufferSkeletonIndex.beginWriting();
		m_engine->getModule_World().updateSystem(
			deltaTime,
			{ Prop_Component::ID, Skeleton_Component::ID },
			{ BaseECSSystem::FLAG_REQUIRED, BaseECSSystem::FLAG_OPTIONAL },
			[&](const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) {
			updateVisibility(deltaTime, components);
		});

		// Apply occlusion culling and render props
		renderGeometry(deltaTime);
		m_bufferPropIndex.endWriting();
		m_bufferCulling.endWriting();
		m_bufferRender.endWriting();
		m_bufferSkeletonIndex.endWriting();
	}


	// Public Methods
	/** Set critical information relating to the position and buffer indicies for the next draw-call. 
	@param	lightPosition		the position of the light.
	@param	lightIndex			the buffer index for the light source. */
	inline void setData(const glm::vec3 & lightPosition, const int & lightIndex) {
		m_lightPos = lightPosition;
		m_lightIndex = lightIndex;
	}


private:
	// Private Methods
	/***/
	inline void updateVisibility(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) {
		// Accumulate draw parameter information per model
		std::vector<glm::ivec4> cullingDrawData, renderingDrawData;
		std::vector<GLuint> visibleIndices;
		std::vector<int> skeletonData;
		const bool renderStatic = m_flags & RenderStatic;
		const bool renderDynamic = m_flags & RenderDynamic;
		for each (const auto & componentParam in components) {
			Prop_Component * propComponent = (Prop_Component*)componentParam[0];
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[1];
			if (!propComponent->m_model->existsYet())
				continue; // Skip if prop isn't ready
			if (renderDynamic && !renderStatic && propComponent->m_static)
				continue; // Skip if we aren't supporting static props	
			else if (renderStatic && !renderDynamic && !propComponent->m_static)
				continue; // Skip if we aren't supporting dynamic props		

			const auto & offset = propComponent->m_model->m_offset;
			const auto & count = propComponent->m_model->m_count;
			const auto & index = *propComponent->m_propBufferIndex;
			visibleIndices.push_back((GLuint)index);
			// Flag for occlusion culling if mesh complexity is high enough and if viewer is NOT within BSphere
			if ((count >= 100) && propComponent->m_radius < glm::distance(propComponent->m_position, m_lightPos)) {
				// Allow
				cullingDrawData.push_back(glm::ivec4(36, m_instanceCount, 0, 1));
				renderingDrawData.push_back(glm::ivec4(count, 0, offset, 1));
			}
			else {
				// Skip occlusion culling		
				cullingDrawData.push_back(glm::ivec4(36, 0, 0, 1));
				renderingDrawData.push_back(glm::ivec4(count, m_instanceCount, offset, 1));
			}
			skeletonData.push_back(skeletonComponent ? (GLint)*skeletonComponent->m_skeleBufferIndex : -1); // get skeleton ID if this entity has one
		}

		// Update camera buffers
		m_visProps = (GLsizei)visibleIndices.size();
		m_bufferPropIndex.write(0, sizeof(GLuint) * m_visProps, visibleIndices.data());
		m_bufferCulling.write(0, sizeof(glm::ivec4) * m_visProps, cullingDrawData.data());
		m_bufferRender.write(0, sizeof(glm::ivec4) * m_visProps, renderingDrawData.data());
		m_bufferSkeletonIndex.write(0, sizeof(int) * m_visProps, skeletonData.data());
	}
	/***/
	inline void renderGeometry(const float & deltaTime) {
		m_engine->getManager_Materials().bind();
		m_propView->getPropBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_bufferPropIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
		m_propView->getSkeletonBuffer().bindBufferBase(GL_SHADER_STORAGE_BUFFER, 5);
		m_bufferSkeletonIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);

		// Draw bounding boxes for each model, filling render buffer on successful rasterization
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		m_shaderCull->bind();
		m_shaderCull->setUniform(0, m_lightIndex);
		glBindVertexArray(m_shapeCube->m_vaoID);
		m_bufferCulling.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_bufferRender.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_visProps, 0);
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);

		// Draw geometry using the populated render buffer
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glFrontFace(GL_CW);
		m_shaderShadow->bind();
		m_shaderShadow->setUniform(0, m_lightIndex);
		glBindVertexArray(*m_modelsVAO);
		m_bufferRender.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_visProps, 0);

		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
	}
	/** Clear out the props queued up for rendering. */
	inline void clear() {
		m_visProps = 0;
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Prop_View * m_propView = nullptr;
	glm::vec3 m_lightPos = glm::vec3(0);
	int m_lightIndex = 0;
	unsigned int m_instanceCount = 0;
	unsigned int m_flags = 0;
	Shared_Shader m_shaderCull, m_shaderShadow;
	Shared_Primitive m_shapeCube;
	const GLuint * m_modelsVAO = nullptr;
	GLsizei m_visProps = 0;
	DynamicBuffer m_bufferPropIndex, m_bufferCulling, m_bufferRender, m_bufferSkeletonIndex;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // PROP_SHADOW_H