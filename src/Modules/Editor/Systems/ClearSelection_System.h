#pragma once
#ifndef CLEARSELECTION_SYSTEM_H
#define CLEARSELECTION_SYSTEM_H 

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Engine.h"


/** An ECS system responsible for deleting all Selected Components from entities. */
class ClearSelection_System : public ecsBaseSystem {
public:
	// Public (de)Constructors
	/** Destroy this system. */
	inline ~ClearSelection_System() = default;
	/** Construct this system.
	@param	engine		the currently active engine. */
	inline ClearSelection_System(Engine* engine)
		: m_engine(engine) {
		// Declare component types used
		addComponentType(Selected_Component::m_ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) override {
		auto& ecsWorld = m_engine->getModule_ECS().getWorld();
		for each (const auto & componentParam in components) 
			ecsWorld.removeComponent<Selected_Component>(((Selected_Component*)(componentParam[0]))->m_entity);
	}


private:
	// Private Attributes
	Engine* m_engine = nullptr;
};

#endif // CLEARSELECTION_SYSTEM_H