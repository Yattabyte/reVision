#pragma once
#ifndef INSPECTOR_COLLIDER_SYSTEM_H
#define INSPECTOR_COLLIDER_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
class Engine;
class LevelEditor_Module;

/** An ECS system allowing the user to inspect selected collider components.*/
class Inspector_Collider_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~Inspector_Collider_System() noexcept = default;
	/** Construct this system. 
	@param	engine		reference to the engine to use. 
	@param	editor		reference to the level-editor to use. */
	Inspector_Collider_System(Engine& engine, LevelEditor_Module& editor);


	// Public Interface Implementation
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Methods
	/** Populates an internal list of models found & supported recursively within the application's models directory. */
	void populateModels();


	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
	std::vector<std::string> m_entries;
};

#endif // INSPECTOR_COLLIDER_SYSTEM_H