#pragma once
#ifndef ENTITYINSPECTOR_H
#define ENTITYINSPECTOR_H

#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/ECS/ecsSystem.h"


/** A level editor UI element responsible for allowing the user to inspect selected entity components. */
class EntityInspector final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Construct a component inspector.
	@param	engine		reference to the engine to use. 
	@param	editor		reference to the level-editor to use. */
	EntityInspector(Engine& engine, LevelEditor_Module& editor) noexcept;


	// Public Interface Implementation
	void tick(const float& deltaTime) final;


private:
	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
	ecsSystemList m_inspectorSystems;
};

#endif // ENTITYINSPECTOR_H