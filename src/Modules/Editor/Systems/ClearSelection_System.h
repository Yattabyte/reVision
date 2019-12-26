#pragma once
#ifndef CLEARSELECTION_SYSTEM_H
#define CLEARSELECTION_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
class Engine;
class LevelEditor_Module;

/** An ECS system responsible for deleting all Selected Components from entities. */
class ClearSelection_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Construct this system.
	@param	editor		reference to the level-editor to use. */
	ClearSelection_System(LevelEditor_Module& editor);


	// Public Interface Implementation
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Attributes
	LevelEditor_Module& m_editor;
};

#endif // CLEARSELECTION_SYSTEM_H