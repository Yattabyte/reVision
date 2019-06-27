#pragma once
#ifndef VIEWINGPERSPECTIVE_SYSTEM_H
#define VIEWINGPERSPECTIVE_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/***/
class ViewingPerspective_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~ViewingPerspective_System() = default;
	/***/
	inline ViewingPerspective_System(const std::shared_ptr<std::vector<Viewport*>> & viewports)
		: m_viewports(viewports) {
		addComponentType(Viewport_Component::ID, FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		m_viewports->clear();
		for each (const auto & componentParam in components) {
			Viewport_Component * viewportComponent = (Viewport_Component*)componentParam[0];
			m_viewports->push_back(viewportComponent->m_camera.get());
		}
	}


private:
	// Private Attributes
	std::shared_ptr<std::vector<Viewport*>> m_viewports;
};

#endif // VIEWINGPERSPECTIVE_SYSTEM_H