#pragma once
#ifndef PROPSHADOWING_S_H
#define PROPSHADOWING_S_H

#include "ECS\Systems\ecsSystem.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Engine.h"
#include <vector>

/* Component Types Used */
#include "ECS\Components\Prop_C.h"
#include "ECS\Components\BoundingSphere_C.h"
#include "ECS\Components\Skeleton_C.h"



class PropShadowing_System : public BaseECSSystem {
public: 
	// Public Enumerations
	enum RenderType_Flags {
		RenderStatic = 0b0000'0001,
		RenderDynamic = 0b0000'0010,
		RenderAll = 0b0000'0011,
	};


	// (de)Constructors
	~PropShadowing_System() {
		if (m_shapeCube.get()) m_shapeCube->removeCallback(this);
		glDeleteVertexArrays(1, &m_cubeVAO);
	}
	PropShadowing_System(Engine * engine, const unsigned int & instanceCount, const unsigned int & flags, Shared_Asset_Shader & shaderCull, Shared_Asset_Shader & shaderShadow, GL_Vector * propBuffer, GL_Vector * skeletonBuffer) : BaseECSSystem() {
		// Declare component types used
		addComponentType(Prop_Component::ID);
		addComponentType(BoundingSphere_Component::ID);
		addComponentType(Skeleton_Component::ID, FLAG_OPTIONAL);

		// Shared Parameters
		m_engine = engine;
		m_instanceCount = instanceCount;
		m_flags = flags;
		m_propBuffer = propBuffer;
		m_skeletonBuffer = skeletonBuffer;
		m_shaderCull = shaderCull;
		m_shaderShadow = shaderShadow;
		m_modelsVAO = &m_engine->getModelManager().getVAO();

		// Asset Loading
		m_shapeCube = Asset_Primitive::Create(engine, "cube");

		// Primitive Construction
		m_cubeVAOLoaded = false;
		m_cubeVAO = Asset_Primitive::Generate_VAO();
		m_shapeCube->addCallback(this, [&]() mutable {
			m_cubeVAOLoaded = true;
			m_shapeCube->updateVAO(m_cubeVAO);
		});
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) {
		// Exit Early
		if (!m_cubeVAOLoaded)
			return;

		// Clear Data
		cullingDrawData.clear();
		renderingDrawData.clear();
		visibleIndices.clear();
		skeletonData.clear();

		// Accumulate draw parameter information per model
		const bool renderStatic = m_flags & RenderStatic;
		const bool renderDynamic = m_flags & RenderDynamic;
		const glm::vec3 & eyePosition = m_engine->getGraphicsModule().m_cameraBuffer.getElement(m_engine->getGraphicsModule().getActiveCamera())->data->EyePosition;
		for each (const auto & componentParam in components) {
			Prop_Component * propComponent = (Prop_Component*)componentParam[0];
			BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[1];
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[2];			
			if (!propComponent->m_model->existsYet())
				continue; // Skip if prop isn't ready
			if (skeletonComponent == nullptr && renderDynamic && !renderStatic)
				continue; // Skip if we aren't supporting static props	
			else if (skeletonComponent && renderStatic && !renderDynamic)
				continue; // Skip if we aren't supporting dynamic props		

			const GLuint & offset = propComponent->m_model->m_offset;
			const GLuint & count = propComponent->m_model->m_count;
			const GLuint & index = propComponent->m_data->index;
			visibleIndices.push_back(index);
			// Flag for occlusion culling  if mesh complexity is high enough and if viewer is NOT within BSphere
			if ((count >= 100) && !(bsphereComponent->m_radius > glm::distance(bsphereComponent->m_position, eyePosition))) { // Allow
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
		const size_t & size = visibleIndices.size();
		m_bufferPropIndex.write(0, sizeof(GLuint) * size, visibleIndices.data());
		m_bufferCulling.write(0, sizeof(glm::ivec4) * size, cullingDrawData.data());
		m_bufferRender.write(0, sizeof(glm::ivec4) * size, renderingDrawData.data());		
		m_bufferSkeletonIndex.write(0, sizeof(int) * size, skeletonData.data());				
		
		m_propBuffer->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 3);
		m_bufferPropIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 4);
		m_skeletonBuffer->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 5);
		m_bufferSkeletonIndex.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 6);

		// Draw bounding boxes for each model, filling render buffer on successful rasterization
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		m_shaderCull->bind();
		glBindVertexArray(m_cubeVAO);
		m_bufferCulling.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_bufferRender.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 7);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, size, 0);
		glMemoryBarrier(GL_COMMAND_BARRIER_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0);

		// Draw geometry using the populated render buffer
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		m_shaderShadow->bind();
		glBindVertexArray(*m_modelsVAO);
		m_bufferRender.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, size, 0);

		glCullFace(GL_BACK);
	}
	

	// Public Attributes
	GL_Vector * m_propBuffer, *m_skeletonBuffer;


private:
	// Private Attributes
	Engine * m_engine;
	unsigned int m_instanceCount;
	unsigned int m_flags;
	Shared_Asset_Shader	m_shaderCull, m_shaderShadow;
	Shared_Asset_Primitive m_shapeCube;
	bool m_cubeVAOLoaded;
	GLuint m_cubeVAO;
	const GLuint * m_modelsVAO;
	std::vector<glm::ivec4> cullingDrawData;
	std::vector<glm::ivec4> renderingDrawData;
	std::vector<GLuint> visibleIndices;
	std::vector<int> skeletonData;
	DynamicBuffer m_bufferPropIndex, m_bufferCulling, m_bufferRender, m_bufferSkeletonIndex;
};

#endif // PROPSHADOWING_S_H