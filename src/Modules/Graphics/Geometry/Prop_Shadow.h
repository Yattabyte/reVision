#pragma once
#ifndef PROP_SHADOW_H
#define PROP_SHADOW_H

#include "Modules/Graphics/Graphics_Technique.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Modules/Graphics/Geometry/Prop_View.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Engine.h"
#include <vector>


/***/
class Prop_Shadow : public Graphics_Technique, public BaseECSSystem {
public:
	// Public Enumerations
	const enum RenderType_Flags {
		RenderStatic = 0b0000'0001,
		RenderDynamic = 0b0000'0010,
		RenderAll = 0b0000'0011,
	};


	// Public (de)Constructors
	/** Destructor. */
	inline ~Prop_Shadow() = default;
	/** Constructor. */
	inline Prop_Shadow(Engine * engine, const unsigned int & instanceCount, const unsigned int & flags, const Shared_Shader & shaderCull, const Shared_Shader & shaderShadow, Prop_View * propView)
		: m_engine(engine), m_instanceCount(instanceCount), m_flags(flags), m_shaderCull(shaderCull), m_shaderShadow(shaderShadow), m_propView(propView) {
		// Asset Loading
		m_shapeCube = Shared_Primitive(engine, "cube");
		m_modelsVAO = &m_engine->getManager_Models().getVAO();

		// Declare component types used
		addComponentType(Prop_Component::ID);
		addComponentType(Skeleton_Component::ID, FLAG_OPTIONAL);

		// Error Reporting
		if (!isValid())
			engine->getManager_Messages().error("Invalid ECS System: PropShadowing_System");
	}


	// Public Interface Implementations
	inline virtual void applyEffect(const float & deltaTime) override {
		// Exit Early
		if (!m_enabled || !m_shapeCube->existsYet())
			return;

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
		glCullFace(GL_FRONT);
		glFrontFace(GL_CW);
		m_shaderShadow->bind();
		glBindVertexArray(*m_modelsVAO);
		m_bufferRender.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_propCount, 0);

		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
	}
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		// Accumulate draw parameter information per model
		std::vector<glm::ivec4> cullingDrawData;
		std::vector<glm::ivec4> renderingDrawData;
		std::vector<GLuint> visibleIndices;
		std::vector<int> skeletonData;
		const bool renderStatic = m_flags & RenderStatic;
		const bool renderDynamic = m_flags & RenderDynamic;
		const glm::vec3 & eyePosition = m_engine->getModule_Graphics().getCameraBuffer()->EyePosition;
		for each (const auto & componentParam in components) {
			Prop_Component * propComponent = (Prop_Component*)componentParam[0];
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[1];
			if (!propComponent->m_model->existsYet())
				continue; // Skip if prop isn't ready
			if (skeletonComponent == nullptr && renderDynamic && !renderStatic)
				continue; // Skip if we aren't supporting static props	
			else if (skeletonComponent && renderStatic && !renderDynamic)
				continue; // Skip if we aren't supporting dynamic props		

			const auto & offset = propComponent->m_model->m_offset;
			const auto & count = propComponent->m_model->m_count;
			const GLuint & index = propComponent->m_data->index;
			visibleIndices.push_back(index);
			// Flag for occlusion culling  if mesh complexity is high enough and if viewer is NOT within BSphere
			if ((count >= 100) && !(propComponent->m_radius > glm::distance(propComponent->m_position, eyePosition))) { // Allow
				cullingDrawData.push_back(glm::ivec4(36, m_instanceCount, 0, 1));
				renderingDrawData.push_back(glm::ivec4(count, 0, offset, 1));
			}
			else { // Skip occlusion culling		
				cullingDrawData.push_back(glm::ivec4(36, 0, 0, 1));
				renderingDrawData.push_back(glm::ivec4(count, m_instanceCount, offset, 1));
			}
			skeletonData.push_back(skeletonComponent ? skeletonComponent->m_data->index : -1); // get skeleton ID if this entity has one
		}

		// Update camera buffers
		const GLsizei & size = (GLsizei)visibleIndices.size();
		m_propCount = size;
		m_bufferPropIndex.write(0, sizeof(GLuint) * size, visibleIndices.data());
		m_bufferCulling.write(0, sizeof(glm::ivec4) * size, cullingDrawData.data());
		m_bufferRender.write(0, sizeof(glm::ivec4) * size, renderingDrawData.data());
		m_bufferSkeletonIndex.write(0, sizeof(int) * size, skeletonData.data());
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Prop_View * m_propView = nullptr;
	unsigned int m_instanceCount = 0;
	unsigned int m_flags = 0;
	Shared_Shader m_shaderCull, m_shaderShadow;
	Shared_Primitive m_shapeCube;
	const GLuint * m_modelsVAO = nullptr;
	GLsizei m_propCount = 0;
	DynamicBuffer m_bufferPropIndex, m_bufferCulling, m_bufferRender, m_bufferSkeletonIndex;
};

#endif // PROP_SHADOW_H