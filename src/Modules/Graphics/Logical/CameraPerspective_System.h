#pragma once
#ifndef CAMERAPERSPECTIVE_SYSTEM_H
#define CAMERAPERSPECTIVE_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/***/
class CameraPerspective_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~CameraPerspective_System() = default;
	/***/
	inline CameraPerspective_System(const std::shared_ptr<std::vector<std::shared_ptr<CameraBuffer>>> & cameras)
		: m_cameras(cameras) {
		addComponentType(Camera_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		m_cameras->clear();
		for each (const auto & componentParam in components) {
			Camera_Component * cameraComponent = (Camera_Component*)componentParam[0];
			m_cameras->push_back(cameraComponent->m_camera);
		}
	}


private:
	// Private Attributes
	std::shared_ptr<std::vector<std::shared_ptr<CameraBuffer>>> m_cameras;
};

#endif // CAMERAPERSPECTIVE_SYSTEM_H