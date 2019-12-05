#pragma once
#ifndef INSPECTOR_LIGHT_SYSTEM_H
#define INSPECTOR_LIGHT_SYSTEM_H

#include "Modules/Editor/Editor_M.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Engine.h"


/** An ECS system allowing the user to inspect selected light components. */
class Inspector_Light_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~Inspector_Light_System() = default;
	/** Construct this system. 
	@param	engine		reference to the engine to use. 
	@param	editor		reference to the level-editor to use. */
	Inspector_Light_System(Engine& engine, LevelEditor_Module& editor) noexcept;


	// Public Interface Implementation
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


private:
	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
};

#endif // INSPECTOR_LIGHT_SYSTEM_H