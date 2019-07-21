#pragma once
#ifndef REFLECTORVISIBILITY_SYSTEM_H
#define REFLECTORVISIBILITY_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"


/** An ECS system responsible for populating render lists PER active perspective in a given frame, for all reflector related entities. */
class ReflectorVisibility_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~ReflectorVisibility_System() = default;
	/** Construct this system. 
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline ReflectorVisibility_System(const std::shared_ptr<ReflectorData> & visibility)
		: m_frameData(visibility) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Reflector_Component::ID, FLAG_REQUIRED);
		addComponentType(Transform_Component::ID, FLAG_REQUIRED);
		addComponentType(CameraArray_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		// Compile results PER viewport
		for (int x = 0; x < m_frameData->viewInfo.size(); ++x) {
			auto & viewInfo = m_frameData->viewInfo[x];

			viewInfo.lightIndices.clear();
			int index = 0;
			for each (const auto & componentParam in components) {
				Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
				Reflector_Component * reflectorComponent = (Reflector_Component*)componentParam[1];
				Transform_Component * transformComponent = (Transform_Component*)componentParam[2];
				CameraArray_Component * cameraComponent = (CameraArray_Component*)componentParam[3];

				// Synchronize the component if it is visible
				if (renderableComponent->m_visible[x])
					viewInfo.lightIndices.push_back((GLuint)index);
				index++;
			}
		}
	}


private:
	// Private Attributes
	std::shared_ptr<ReflectorData> m_frameData;
};

#endif // REFLECTORVISIBILITY_SYSTEM_H