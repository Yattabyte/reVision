#pragma once
#ifndef PHYSICSSYNC_SYSTEM_H
#define PHYSICSSYNC_SYSTEM_H
#define GLM_ENABLE_EXPERIMENTAL

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Engine.h"
#include "glm/glm.hpp"
#include "glm/gtx/component_wise.hpp"


/** A system responsible for updating physics components that share a common transformation. */
class PhysicsSync_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this physics sync system. */
	inline ~PhysicsSync_System() = default;
	/** Construct a physics sync system.
	@param	engine		reference to the engine to use.
	@param	world		reference to the physics world to use. */
	PhysicsSync_System(Engine& engine, btDiscreteDynamicsWorld& world) noexcept;


	// Public Interface Implementation
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


private:
	// Private Attributes
	Engine& m_engine;
	btDiscreteDynamicsWorld& m_world;
};

#endif // PHYSICSSYNC_SYSTEM_H