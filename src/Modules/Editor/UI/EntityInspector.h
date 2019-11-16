#pragma once
#ifndef ENTITYINSPECTOR_H
#define ENTITYINSPECTOR_H

#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/ECS/ecsSystem.h"
#include <map>


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element responsible for allowing the user to inspect selected entity components. */
class EntityInspector final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Destroy this inspector. */
	inline ~EntityInspector() = default;
	/** Construct a component inspector.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	EntityInspector(Engine* engine, LevelEditor_Module* editor) noexcept;


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) noexcept override final;


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	ecsSystemList m_inspectorSystems;
};

#endif // ENTITYINSPECTOR_H
