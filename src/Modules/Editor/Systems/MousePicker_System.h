#pragma once
#ifndef MOUSEPICKER_SYSTEM_H
#define MOUSEPICKER_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Utilities/Transform.h"
#include "glm/glm.hpp"


// Forward Declarations
class Engine;

/** An ECS system allowing the user to ray-pick entities by selecting against their components. */
class MousePicker_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	~MousePicker_System() noexcept;
	/** Construct this system.
	@param	engine		reference to the engine to use. */
	explicit MousePicker_System(Engine& engine);


	// Public Interface Implementation
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


	// Public Methods
	/** Retrieve this system's last selection result.
	@return							the last selection result. */
	std::tuple<EntityHandle, Transform, Transform> getSelection() const noexcept;


private:
	// Private Attributes
	Engine& m_engine;
	EntityHandle m_selection;
	Transform m_selectionTransform, m_intersectionTransform;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // MOUSEPICKER_SYSTEM_H