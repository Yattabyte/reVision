#pragma once
#ifndef PROP_VIEW_H
#define PROP_VIEW_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Modules/World/ECS/TransformComponent.h"
#include "Assets/Shader.h"
#include "Assets/Primitive.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "Engine.h"
#include "glm/gtx/component_wise.hpp"
#include <vector>

#define NUM_MAX_BONES 100


/** A core rendering technique for rendering props from a given viewing perspective. */
class Prop_View : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destructor. */
	inline ~Prop_View() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Constructor. */
	inline Prop_View(Engine * engine)
		: m_engine(engine) {		
		// Asset Loading
		m_shaderCull = Shared_Shader(m_engine, "Core\\Props\\culling");
		m_shaderGeometry = Shared_Shader(m_engine, "Core\\Props\\geometry");
		m_shapeCube = Shared_Primitive(m_engine, "cube");
		m_modelsVAO = &m_engine->getManager_Models().getVAO();

		// Declare component types used
		addComponentType(Prop_Component::ID);
		addComponentType(Skeleton_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);

		// Error Reporting
		if (!isValid())
			m_engine->getManager_Messages().error("Invalid ECS System: Prop_View");

		// Add New Component Types
		auto & world = m_engine->getModule_World();
		world.addNotifyOnComponentType(Prop_Component::ID, m_aliveIndicator, [&](BaseECSComponent * c) {
			auto * component = (Prop_Component*)c;
			component->m_propBufferIndex = m_propBuffer.newElement();
			m_propBuffer[*component->m_propBufferIndex].materialID = component->m_skin;
		});
		world.addNotifyOnComponentType(Skeleton_Component::ID, m_aliveIndicator, [&](BaseECSComponent * c) {
			auto * component = (Skeleton_Component*)c;
			component->m_skeleBufferIndex = m_skeletonBuffer.newElement();
			for (int x = 0; x < NUM_MAX_BONES; ++x)
				m_skeletonBuffer[*component->m_skeleBufferIndex].bones[x] = glm::mat4(1.0f);
		});

		// World-Changed Callback
		world.addLevelListener(m_aliveIndicator, [&](const World_Module::WorldState & state) {
			if (state == World_Module::unloaded)
				clear();
		});
	}


	// Public Interface Implementations
	inline virtual void applyTechnique(const float & deltaTime) override {
		// Exit Early
		if (!m_enabled || !m_shapeCube->existsYet() || !m_shaderCull->existsYet() || !m_shaderGeometry->existsYet())
			return;

		m_engine->getManager_Materials().bind();
		m_propBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_bufferPropIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
		m_skeletonBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 5);
		m_bufferSkeletonIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);

		// Draw bounding boxes for each model, filling render buffer on successful rasterization
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		m_shaderCull->bind();
		m_gfxFBOS->bindForWriting("GEOMETRY");
		glBindVertexArray(m_shapeCube->m_vaoID);
		m_bufferCulling.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_bufferRender.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_propCount, 0);
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);

		// Draw geometry using the populated render buffer
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		m_shaderGeometry->bind();
		glBindVertexArray(*m_modelsVAO);
		m_bufferRender.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_propCount, 0);
	}
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		// Accumulate draw parameter information per model
		std::vector<glm::ivec4> cullingDrawData, renderingDrawData;
		std::vector<GLuint> visibleIndices;
		std::vector<int> skeletonData;
		const glm::vec3 & eyePosition = (*m_cameraBuffer)->EyePosition;
		for each (const auto & componentParam in components) {
			Prop_Component * propComponent = (Prop_Component*)componentParam[0];
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[1];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[2];
			const auto & offset = propComponent->m_model->m_offset;
			const auto & count = propComponent->m_model->m_count;
			const auto & index = *propComponent->m_propBufferIndex;
			if (!propComponent->m_model->existsYet())	continue;

			// Sync Transform Attributes
			if (transformComponent) {
				const auto & position = transformComponent->m_transform.m_position;
				const auto & orientation = transformComponent->m_transform.m_orientation;
				const auto & scale = transformComponent->m_transform.m_scale;
				const auto matRot = glm::mat4_cast(orientation);
				propComponent->mMatrix = transformComponent->m_transform.m_modelMatrix;

				// Update bounding sphere
				const glm::vec3 bboxMax_World = (propComponent->m_model->m_bboxMax * scale) + position;
				const glm::vec3 bboxMin_World = (propComponent->m_model->m_bboxMin * scale) + position;
				const glm::vec3 bboxCenter = (bboxMax_World + bboxMin_World) / 2.0f;
				const glm::vec3 bboxScale = (bboxMax_World - bboxMin_World) / 2.0f;
				glm::mat4 matTrans = glm::translate(glm::mat4(1.0f), bboxCenter);
				glm::mat4 matScale = glm::scale(glm::mat4(1.0f), bboxScale);
				glm::mat4 matFinal = (matTrans * matRot * matScale);
				propComponent->bBoxMatrix = matFinal;
				propComponent->m_radius = glm::compMax(propComponent->m_model->m_radius * scale);
				propComponent->m_position = propComponent->m_model->m_bboxCenter + position;
				
			}

			// Sync Animation Attributes
			if (skeletonComponent) {
				propComponent->m_static = false;
				auto & bones = m_skeletonBuffer[*skeletonComponent->m_skeleBufferIndex].bones;
				for (size_t i = 0, total = std::min(skeletonComponent->m_transforms.size(), size_t(NUM_MAX_BONES)); i < total; ++i)
					bones[i] = skeletonComponent->m_transforms[i];
			}

			// Sync Prop Attributes
			m_propBuffer[index].materialID = propComponent->m_skin;
			m_propBuffer[index].isStatic = propComponent->m_static;
			m_propBuffer[index].mMatrix = propComponent->mMatrix;
			m_propBuffer[index].bBoxMatrix = propComponent->bBoxMatrix;

			// Flag for occlusion culling if mesh complexity is high enough and if viewer is NOT within BSphere
			visibleIndices.push_back((GLuint)index);
			if ((count >= 100) && propComponent->m_radius < glm::distance(propComponent->m_position, eyePosition)) { 
				// Allow
				cullingDrawData.push_back(glm::ivec4(36, 1, 0, 1));
				renderingDrawData.push_back(glm::ivec4(count, 0, offset, 1));
			}
			else { 
				// Skip occlusion culling		
				cullingDrawData.push_back(glm::ivec4(36, 0, 0, 1));
				renderingDrawData.push_back(glm::ivec4(count, 1, offset, 1));
			}
			skeletonData.push_back(skeletonComponent ? (GLint)*skeletonComponent->m_skeleBufferIndex : -1); // get skeleton ID if this entity has one
		}

		// Update camera buffers
		m_propCount = (GLsizei)visibleIndices.size();
		m_bufferPropIndex.write(0, sizeof(GLuint) * m_propCount, visibleIndices.data());
		m_bufferCulling.write(0, sizeof(glm::ivec4) * m_propCount, cullingDrawData.data());
		m_bufferRender.write(0, sizeof(glm::ivec4) * m_propCount, renderingDrawData.data());
		m_bufferSkeletonIndex.write(0, sizeof(int) * m_propCount, skeletonData.data());
	}


	// Public Methods
	/** Retrieve the prop buffer.
	@return		the prop buffer. */
	inline auto & getPropBuffer() {
		return m_propBuffer;
	}
	/** Retrieve the skeleton buffer. 
	@return		the skeleton buffer. */
	inline auto & getSkeletonBuffer() {
		return m_skeletonBuffer;
	}


private:
	// Private Methods
	/** Clear out the props queued up for rendering. */
	void clear() {
		m_propCount = 0;
		m_propBuffer.clear();
		m_skeletonBuffer.clear();
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	const GLuint * m_modelsVAO = nullptr;
	Shared_Shader m_shaderCull, m_shaderGeometry;
	Shared_Primitive m_shapeCube;
	GLsizei m_propCount = 0;
	DynamicBuffer m_bufferPropIndex, m_bufferCulling, m_bufferRender, m_bufferSkeletonIndex;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);

	// Core Prop Data
	/** OpenGL buffer for props. */
	struct Prop_Buffer {
		GLuint materialID; 
		GLuint isStatic; glm::vec2 padding1;
		glm::mat4 mMatrix;
		glm::mat4 bBoxMatrix;
	};
	/** OpenGL buffer for prop skeletons. */
	struct Skeleton_Buffer {
		glm::mat4 bones[NUM_MAX_BONES];
	};
	GL_ArrayBuffer<Prop_Buffer> m_propBuffer;
	GL_ArrayBuffer<Skeleton_Buffer> m_skeletonBuffer;
};

#endif // PROP_VIEW_H