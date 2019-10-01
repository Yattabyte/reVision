#pragma once
#ifndef ECS_MODULE_H
#define ECS_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsWorld.h"
#include "Modules/ECS/ecsSystem.h"


/** A module responsible for the engine entities, components, and systems. */
class ECS_Module final : public Engine_Module {
public:
	// Public (de)Constructors
	/** Destroy this ECS module. */
	inline ~ECS_Module() = default;
	/** Construct a ECS module. */
	inline ECS_Module() = default;


	// Public Interface Implementations
	virtual void initialize(Engine* engine) override final;
	virtual void deinitialize() override final;
	virtual void frameTick(const float& deltaTime) override final;


	// Public Methods
	/** Retrieve the currently active ECS-world.
	@return			the current ECS-world.*/
	inline ecsWorld& getWorld() {
		return m_world;
	}
	/** Overwrite the currently active ECS-world.
	@param			the new ECS-world to use. */
	inline void setWorld(ecsWorld&& newWorld) {
		m_world = std::move(newWorld);
	}
	/** Update the components of all systems provided.
	@param	systems				the systems to update.
	@param	deltaTime			the delta time. */
	void updateSystems(ecsSystemList& systems, const float& deltaTime);
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	void updateSystem(ecsBaseSystem* system, const float& deltaTime);
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	void updateSystem(const std::shared_ptr<ecsBaseSystem>& system, const float& deltaTime);
	/** Update the components of a single system.
	@param	deltaTime			the delta time.
	@param	componentTypes		list of component types to retrieve.
	@param	func				lambda function serving as a system. */
	void updateSystem(const float& deltaTime, const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes, const std::function<void(const float&, const std::vector<std::vector<ecsBaseComponent*>>&)>& func);


private:
	// Private Attributes
	ecsWorld m_world;
};

#endif // ECS_MODULE_H