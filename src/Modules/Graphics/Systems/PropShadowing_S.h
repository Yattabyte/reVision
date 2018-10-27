#pragma once
#ifndef PROPSHADOWING_S_H
#define PROPSHADOWING_S_H

#include "Utilities\ECS\ecsSystem.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Engine.h"
#include <vector>

/* Component Types Used */
#include "Modules\Graphics\Components\Prop_C.h"
#include "Modules\Graphics\Components\Skeleton_C.h"


/** A struct that holds rendering data that can change frame-to-frame. */
struct PropShadow_RenderState {
	GLsizei m_propCount = 0;
	DynamicBuffer m_bufferPropIndex, m_bufferCulling, m_bufferRender, m_bufferSkeletonIndex;
};

/** A core rendering effect which renders props to a shadow map. */
class PropShadowing_System : public BaseECSSystem {
public: 
	// Public Enumerations
	enum RenderType_Flags {
		RenderStatic = 0b0000'0001,
		RenderDynamic = 0b0000'0010,
		RenderAll = 0b0000'0011,
	};


	// (de)Constructors
	~PropShadowing_System() = default;
	PropShadowing_System(
		Engine * engine, const unsigned int & instanceCount, const unsigned int & flags
	) : m_engine(engine), m_instanceCount(instanceCount), m_flags(flags) {
		// Declare component types used
		addComponentType(Prop_Component::ID);
		addComponentType(Skeleton_Component::ID, FLAG_OPTIONAL);
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		// Accumulate draw parameter information per model
		std::vector<glm::ivec4> cullingDrawData;
		std::vector<glm::ivec4> renderingDrawData;
		std::vector<GLuint> visibleIndices;
		std::vector<int> skeletonData;
		const bool renderStatic = m_flags & RenderStatic;
		const bool renderDynamic = m_flags & RenderDynamic;
		const glm::vec3 & eyePosition = m_engine->getGraphicsModule().getActiveCameraBuffer()->data->EyePosition;
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
		m_renderState.m_propCount = size;
		m_renderState.m_bufferPropIndex.write(0, sizeof(GLuint) * size, visibleIndices.data());
		m_renderState.m_bufferCulling.write(0, sizeof(glm::ivec4) * size, cullingDrawData.data());
		m_renderState.m_bufferRender.write(0, sizeof(glm::ivec4) * size, renderingDrawData.data());
		m_renderState.m_bufferSkeletonIndex.write(0, sizeof(int) * size, skeletonData.data());
	}
	

	// Public Attributes
	PropShadow_RenderState m_renderState;


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	unsigned int m_instanceCount;
	unsigned int m_flags;
};

#endif // PROPSHADOWING_S_H