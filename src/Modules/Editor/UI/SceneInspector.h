#pragma once
#ifndef SCENEINSPECTOR_H
#define SCENEINSPECTOR_H

#include "Modules/UI/UI_M.h"
#include "Modules/World/ECS/ecsSystem.h"
#include <map>


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element responsible for allowing the user to see all the elements of the active scene. */
class SceneInspector : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy this scene inspector. */
	inline ~SceneInspector() = default;
	/** Construct a scene inspector.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	SceneInspector(Engine * engine, LevelEditor_Module * editor);


	// Public Interface Implementation
	virtual void tick(const float & deltaTime) override;


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	LevelEditor_Module * m_editor = nullptr;
};

#endif // SCENEINSPECTOR_H