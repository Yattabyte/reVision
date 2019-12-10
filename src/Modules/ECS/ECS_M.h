#pragma once
#ifndef ECS_MODULE_H
#define ECS_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsWorld.h"
#include "Modules/ECS/ecsSystem.h"


/** A module responsible for the engine entities, components, and systems. */
class ECS_Module final : public Engine_Module {
public:
	// Public (De)Constructors
	/** Destroy this ECS module. */
	inline ~ECS_Module() noexcept = default;
	/** Construct a ECS module.
	@param	engine		reference to the engine to use. */
	explicit ECS_Module(Engine& engine) noexcept;


	// Public Interface Implementations
	void initialize() noexcept final;
	void deinitialize() noexcept final;


	// Public Methods
	/** Update the components of all systems provided.
	@param	systems				the systems to update.
	@param	world				the ecsWorld to source data from.
	@param	deltaTime			the delta time. */
	[[maybe_unused]] static void updateSystems(ecsSystemList& systems, ecsWorld& world, const float& deltaTime = 0.0f) noexcept;
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	world				the ecsWorld to source data from.
	@param	deltaTime			the delta time. */
	[[maybe_unused]] static void updateSystem(ecsBaseSystem* system, ecsWorld& world, const float& deltaTime = 0.0f) noexcept;
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	world				the ecsWorld to source data from.
	@param	deltaTime			the delta time. */
	[[maybe_unused]] static void updateSystem(const std::shared_ptr<ecsBaseSystem>& system, ecsWorld& world, const float& deltaTime = 0.0f) noexcept;
	/** Update the components of a single system.
	@param	deltaTime			the delta time.
	@param	world				the ecsWorld to source data from.
	@param	componentTypes		list of component types to retrieve.
	@param	func				lambda function serving as a system. */
	[[maybe_unused]] static void updateSystem(const float& deltaTime, ecsWorld& world, const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes, const std::function<void(const float&, const std::vector<std::vector<ecsBaseComponent*>>&)>& func) noexcept;
};

#endif // ECS_MODULE_H