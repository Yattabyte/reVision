#pragma once
#ifndef CLEARSELECTION_SYSTEM_H
#define CLEARSELECTION_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Editor/Editor_M.h"
#include "Engine.h"


/** An ECS system responsible for deleting all Selected Components from entities. */
class ClearSelection_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~ClearSelection_System() = default;
	/** Construct this system.
	@param	engine		reference to the engine to use. */
	inline ClearSelection_System(Engine& engine, LevelEditor_Module& editor) noexcept :
		m_engine(engine),
		m_editor(editor)
	{
		// Declare component types used
		addComponentType(Selected_Component::Runtime_ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final {
		auto& ecsWorld = m_editor.getWorld();
		for (const auto& componentParam : components)
			ecsWorld.removeEntityComponent((static_cast<Selected_Component*>(componentParam[0]))->m_entity, Selected_Component::Runtime_ID);
	}


private:
	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
};

#endif // CLEARSELECTION_SYSTEM_H
