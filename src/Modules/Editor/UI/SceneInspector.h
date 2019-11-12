#pragma once
#ifndef SCENEINSPECTOR_H
#define SCENEINSPECTOR_H

#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/ECS/ecsSystem.h"
#include <map>


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element responsible for allowing the user to see all the elements of the active scene. */
class SceneInspector final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Destroy this scene inspector. */
	inline ~SceneInspector() = default;
	/** Construct a scene inspector.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	SceneInspector(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override final;


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
};

#endif // SCENEINSPECTOR_H