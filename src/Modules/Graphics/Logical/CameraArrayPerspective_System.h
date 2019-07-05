#pragma once
#ifndef CAMERAARRAYPERSPECTIVE_SYSTEM_H
#define CAMERAARRAYPERSPECTIVE_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/***/
class CameraArrayPerspective_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~CameraArrayPerspective_System() = default;
	/***/
	inline CameraArrayPerspective_System(const std::shared_ptr<std::vector<CameraBuffer::CamStruct*>> & cameras)
		: m_cameras(cameras) {
		addComponentType(CameraArray_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		for each (const auto & componentParam in components) {
			CameraArray_Component * cameraComponent = (CameraArray_Component*)componentParam[0];
			if (cameraComponent->m_cameras.size())
				for (auto & camera : cameraComponent->m_cameras)
					m_cameras->push_back(&camera);
		}
	}


private:
	// Private Attributes
	std::shared_ptr<std::vector<CameraBuffer::CamStruct*>> m_cameras;
};

#endif // CAMERAARRAYPERSPECTIVE_SYSTEM_H