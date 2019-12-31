#pragma once
#ifndef INSPECTOR_LIGHT_SYSTEM_H
#define INSPECTOR_LIGHT_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
class Engine;
class LevelEditor_Module;

/** An ECS system allowing the user to inspect selected light components. */
class Inspector_Light_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Construct this system.
	@param	engine		reference to the engine to use.
	@param	editor		reference to the level-editor to use. */
	Inspector_Light_System(Engine& engine, LevelEditor_Module& editor);


	// Public Interface Implementation
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
};

#endif // INSPECTOR_LIGHT_SYSTEM_H