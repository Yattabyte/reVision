#pragma once
#ifndef REFLECTOR_S_H
#define REFLECTOR_S_H 

#include "Utilities/ECS/ecsSystem.h"
#include "Modules/Graphics/ECS/Reflector_C.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/PriorityList.h"
#include "Engine.h"


/** A struct that holds rendering data that can change frame-to-frame. */
struct Reflector_RenderState {
	StaticBuffer m_indirectCube = StaticBuffer(sizeof(GLuint) * 4), m_indirectQuad = StaticBuffer(sizeof(GLuint) * 4), m_indirectQuad6Faces = StaticBuffer(sizeof(GLuint) * 4);
	GLuint m_envmapSize = 512u;
	DynamicBuffer m_visLights;
	std::vector<Reflector_Component*> m_reflectorsToUpdate;
	bool m_outOfDate = true;
};

/** A system that updates the rendering state for local reflectors, using the ECS system. */
class Reflector_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Reflector_System() = default;
	Reflector_System() {
		// Declare component types used
		addComponentType(Reflector_Component::ID);
		GLuint data[] = { 0,0,0,0 };
		m_renderState.m_indirectCube.write(0, sizeof(GLuint) * 4, &data);
		m_renderState.m_indirectQuad.write(0, sizeof(GLuint) * 4, &data);
		m_renderState.m_indirectQuad6Faces.write(0, sizeof(GLuint) * 4, &data);
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		// Accumulate Reflector Data		
		std::vector<GLint> reflectionIndicies;
		PriorityList<float, Reflector_Component*, std::less<float>> oldest;
		for each (const auto & componentParam in components) {
			Reflector_Component * component = (Reflector_Component*)componentParam[0];
			reflectionIndicies.push_back(component->m_data->index);
			oldest.insert(component->m_updateTime, component);
		}

		// Update Draw Buffers
		const size_t & refSize = reflectionIndicies.size();
		m_renderState.m_visLights.write(0, sizeof(GLuint) *refSize, reflectionIndicies.data());
		m_renderState.m_indirectCube.write(sizeof(GLuint), sizeof(GLuint), &refSize); // update primCount (2nd param)
		m_renderState.m_reflectorsToUpdate = PQtoVector(oldest);
	}


	// Public Attributes
	Reflector_RenderState m_renderState;


private:
	// Private methods
	/** Converts a priority queue into an stl vector.*/
	const std::vector<Reflector_Component*> PQtoVector(PriorityList<float, Reflector_Component*, std::less<float>> oldest) const {
		PriorityList<float, Reflector_Component*, std::greater<float>> m_closest(2);
		std::vector<Reflector_Component*> outList;
		outList.reserve(2);

		for each (const auto &element in oldest.toList()) {
			if (outList.size() < 2)
				outList.push_back(element);
			else
				m_closest.insert(element->m_updateTime, element);
		}

		for each (const auto &element in m_closest.toList()) {
			if (outList.size() >= 2)
				break;
			outList.push_back(element);
		}

		return outList;
	}
};

#endif // REFLECTOR_S_H