#pragma once
#ifndef LIGHTINGPOINT_S_H
#define LIGHTINGPOINT_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Modules\Graphics\Components\LightPoint_C.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\DynamicBuffer.h"
#include "Utilities\PriorityList.h"
#include "Engine.h"
#include <vector>


/** A struct that holds rendering data that can change frame-to-frame. */
struct Point_RenderState {
	GLuint m_updateQuality = 1u;
	glm::ivec2 m_shadowSize = glm::ivec2(512);
	StaticBuffer m_indirectShape = StaticBuffer(sizeof(GLuint) * 4);	
	DynamicBuffer m_visLights, m_visShadows;
	std::vector<std::pair<LightPoint_Component*, LightPointShadow_Component*>> m_shadowsToUpdate;
	bool m_outOfDate = true;
};

/** A system that updates the rendering state for point lighting, using the ECS system. */
class LightPoint_System : public BaseECSSystem {
public: 
	// (de)Constructors
	~LightPoint_System() = default;
	LightPoint_System() {
		// Declare component types used
		addComponentType(LightPoint_Component::ID);
		addComponentType(LightPointShadow_Component::ID, FLAG_OPTIONAL);
		GLuint data[] = { 0,0,0,0 };
		m_renderState.m_indirectShape.write(0, sizeof(GLuint) * 4, &data);
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		// Accumulate Light Data	
		std::vector<GLint> lightIndices, shadowIndices;
		PriorityList<float, std::pair<LightPoint_Component*, LightPointShadow_Component*>, std::less<float>> oldest;
		for each (const auto & componentParam in components) {
			LightPoint_Component * lightComponent = (LightPoint_Component*)componentParam[0];
			LightPointShadow_Component * shadowComponent = (LightPointShadow_Component*)componentParam[1];
			lightIndices.push_back(lightComponent->m_data->index);
			if (shadowComponent) {
				shadowIndices.push_back(shadowComponent->m_data->index);
				oldest.insert(shadowComponent->m_updateTime, std::make_pair(lightComponent, shadowComponent));	
			}
			else
				shadowIndices.push_back(-1);
		}

		// Update Draw Buffers
		const size_t & lightSize = lightIndices.size();
		m_renderState.m_visLights.write(0, sizeof(GLuint) * lightSize, lightIndices.data());
		m_renderState.m_visShadows.write(0, sizeof(GLuint) * shadowIndices.size(), shadowIndices.data());
		m_renderState.m_indirectShape.write(sizeof(GLuint), sizeof(GLuint), &lightSize); // update primCount (2nd param)		
		m_renderState.m_shadowsToUpdate = PQtoVector(oldest);
	}


	// Public Attributes
	Point_RenderState m_renderState;


private:
	// Private methods
	/** Converts a priority queue into an stl vector.*/
	const std::vector<std::pair<LightPoint_Component*, LightPointShadow_Component*>> PQtoVector(PriorityList<float, std::pair<LightPoint_Component*, LightPointShadow_Component*>, std::less<float>> oldest) const {
		PriorityList<float, std::pair<LightPoint_Component*, LightPointShadow_Component*>, std::greater<float>> m_closest(m_renderState.m_updateQuality / 2);
		std::vector<std::pair<LightPoint_Component*, LightPointShadow_Component*>> outList;
		outList.reserve(m_renderState.m_updateQuality);
		for each (const auto &element in oldest.toList()) {
			if (outList.size() < (m_renderState.m_updateQuality / 2))
				outList.push_back(element);
			else
				m_closest.insert(element.second->m_updateTime, element);
		}

		for each (const auto &element in m_closest.toList()) {
			if (outList.size() >= m_renderState.m_updateQuality)
				break;
			outList.push_back(element);
		}

		return outList;
	}
};

#endif // LIGHTINGPOINT_S_H