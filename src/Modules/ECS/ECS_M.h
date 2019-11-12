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
	inline ~ECS_Module() = default;
	/** Construct a ECS module. */
	inline ECS_Module() = default;


	// Public Interface Implementations
	virtual void initialize(Engine* engine) override final;
	virtual void deinitialize() override final;


	// Public Methods
	/** Update the components of all systems provided.
	@param	systems				the systems to update.
	@param	deltaTime			the delta time. */
	[[maybe_unused]] static void updateSystems(ecsSystemList& systems, ecsWorld& world, const float& deltaTime = 0.0f);
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	[[maybe_unused]] static void updateSystem(ecsBaseSystem* system, ecsWorld& world, const float& deltaTime = 0.0f);
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	[[maybe_unused]] static void updateSystem(const std::shared_ptr<ecsBaseSystem>& system, ecsWorld& world, const float& deltaTime = 0.0f);
	/** Update the components of a single system.
	@param	deltaTime			the delta time.
	@param	componentTypes		list of component types to retrieve.
	@param	func				lambda function serving as a system. */
	[[maybe_unused]] static void updateSystem(const float& deltaTime, ecsWorld& world, const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes, const std::function<void(const float&, const std::vector<std::vector<ecsBaseComponent*>>&)>& func);
};

#endif // ECS_MODULE_H