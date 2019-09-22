#pragma once
#ifndef REFLECTORVISIBILITY_SYSTEM_H
#define REFLECTORVISIBILITY_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"


/** An ECS system responsible for populating render lists PER active perspective in a given frame, for all reflector related entities. */
class ReflectorVisibility_System : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~ReflectorVisibility_System() = default;
	/** Construct this system. 
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline ReflectorVisibility_System(const std::shared_ptr<ReflectorData> & visibility)
		: m_frameData(visibility) {
		addComponentType(Reflector_Component::m_ID, FLAG_REQUIRED);
		addComponentType(Transform_Component::m_ID, FLAG_REQUIRED);
		addComponentType(CameraArray_Component::m_ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<ecsBaseComponent*>> & components) override {
		// Compile results PER viewport
		for (int x = 0; x < m_frameData->viewInfo.size(); ++x) {
			auto & viewInfo = m_frameData->viewInfo[x];

			viewInfo.lightIndices.clear();
			int index = 0;
			for each (const auto & componentParam in components) {
				auto* reflectorComponent = (Reflector_Component*)componentParam[0];
				auto* transformComponent = (Transform_Component*)componentParam[1];
				auto* cameraComponent = (CameraArray_Component*)componentParam[2];

				// Synchronize the component if it is visible
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