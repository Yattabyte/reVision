#pragma once
#ifndef REFLECTORPERSPECTIVE_SYSTEM_H
#define REFLECTORPERSPECTIVE_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/** An ECS system responsible for updating a shared pointer with a list of all active cameras from reflectors in the scene. */
class ReflectorPerspective_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~ReflectorPerspective_System() = default;
	/** Construct this system.
	@param	sceneCameras	reference to the scene cameras to use. */
	explicit ReflectorPerspective_System(std::vector<Camera*>& sceneCameras) noexcept;


	// Public Interface Implementations
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


private:
	// Private Attributes
	std::vector<Camera*>& m_sceneCameras;
};

#endif // REFLECTORPERSPECTIVE_SYSTEM_H