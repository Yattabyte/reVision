#pragma once
#ifndef CAMERAFOLLOWER_SYSTEM_H
#define CAMERAFOLLOWER_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Common/components.h"
#include "Modules/Graphics/Common/Viewport.h"
#include <memory>


/***/
class CameraFollower_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~CameraFollower_System() = default;
	/***/
	inline CameraFollower_System() {
		addComponentType(CameraFollower_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		for each (const auto & componentParam in components) 
			((CameraFollower_Component*)componentParam[0])->m_viewport = m_viewport;		
	}


	// Public Methods
	/***/
	void setViewport(const std::shared_ptr<Viewport> & viewport) {
		m_viewport = viewport;
	}


private:
	// Private Attributes
	std::shared_ptr<Viewport> m_viewport;
};

#endif // CAMERAFOLLOWER_SYSTEM_H