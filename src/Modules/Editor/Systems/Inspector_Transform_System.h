#pragma once
#ifndef INSPECTOR_TRANSFORM_SYSTEM_H
#define INSPECTOR_TRANSFORM_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
class Engine;
class LevelEditor_Module;

/** An ECS system allowing the user to inspect selected component transforms. */
class Inspector_Transform_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~Inspector_Transform_System() noexcept = default;
	/** Construct this system.
	@param	engine		reference to the engine to use. 
	@param	editor		reference to the level-editor to use. */
	Inspector_Transform_System(Engine& engine, LevelEditor_Module& editor) noexcept;


	// Public Interface Implementation
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
};

#endif // INSPECTOR_TRANSFORM_SYSTEM_H