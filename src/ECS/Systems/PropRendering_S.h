#pragma once
#ifndef PROPRENDERING_S_H
#define PROPRENDERING_S_H

#include "ECS\Systems\ecsSystem.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\FBO.h"
#include "Engine.h"
#include <vector>

/* Component Types Used */
#include "ECS\Components\Prop_C.h"
#include "ECS\Components\BoundingSphere_C.h"
#include "ECS\Components\Skeleton_C.h"

/** A struct that holds rendering data that can change frame-to-frame. */
struct Prop_RenderState {
	GLsizei m_propCount = 0;
	DynamicBuffer m_bufferPropIndex, m_bufferCulling, m_bufferRender, m_bufferSkeletonIndex;
};

/** A core rendering effect which renders props to the scene. */
class PropRendering_System : public BaseECSSystem {
public: 
	// (de)Constructors
	~PropRendering_System() = default;
	PropRendering_System(
		Engine * engine
	) : m_engine(engine) {
		// Declare component types used
		addComponentType(Prop_Component::ID);
		addComponentType(BoundingSphere_Component::ID);
		addComponentType(Skeleton_Component::ID, FLAG_OPTIONAL);
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		// Accumulate draw parameter information per model
		std::vector<glm::ivec4> cullingDrawData, renderingDrawData;
		std::vector<GLuint> visibleIndices;
		std::vector<int> skeletonData;
		const glm::vec3 & eyePosition = m_engine->getGraphicsModule().m_cameraBuffer.getElement(m_engine->getGraphicsModule().getActiveCamera())->data->EyePosition;
		for each (const auto & componentParam in components) {
			Prop_Component * propComponent = (Prop_Component*)componentParam[0];
			BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[1];
			Skeleton_Component * skeletonComponent = (Skeleton_Component*)componentParam[2];
			if (!propComponent->m_model->existsYet())
				continue;

			const auto & offset = propComponent->m_model->m_offset;
			const auto & count = propComponent->m_model->m_count;
			const GLuint & index = propComponent->m_data->index;
			visibleIndices.push_back(index);
			// Flag for occlusion culling  if mesh complexity is high enough and if viewer is NOT within BSphere
			if ((count >= 100) && !(bsphereComponent->m_radius > glm::distance(bsphereComponent->m_position, eyePosition))) { // Allow
				cullingDrawData.push_back(glm::ivec4(36, 1, 0, 1));
				renderingDrawData.push_back(glm::ivec4(count, 0, offset, 1));
			}
			else { // Skip occlusion culling		
				cullingDrawData.push_back(glm::ivec4(36, 0, 0, 1));
				renderingDrawData.push_back(glm::ivec4(count, 1, offset, 1));
			}
			skeletonData.push_back(skeletonComponent ? skeletonComponent->m_data->index : -1); // get skeleton ID if this entity has one
		}
		
		// Update camera buffers
		const GLsizei size = (GLsizei)visibleIndices.size();
		m_renderState.m_propCount = size;
		m_renderState.m_bufferPropIndex.write(0, sizeof(GLuint) * size, visibleIndices.data());
		m_renderState.m_bufferCulling.write(0, sizeof(glm::ivec4) * size, cullingDrawData.data());
		m_renderState.m_bufferRender.write(0, sizeof(glm::ivec4) * size, renderingDrawData.data());
		m_renderState.m_bufferSkeletonIndex.write(0, sizeof(int) * size, skeletonData.data());
	}
	

	// Public Attributes
	Prop_RenderState m_renderState;


private:
	// Private Attributes
	Engine * m_engine;
};

#endif // PROPRENDERING_S_H