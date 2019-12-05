#pragma once
#ifndef SCENEINSPECTOR_H
#define SCENEINSPECTOR_H

#include "Modules/Editor/UI/Editor_Interface.h"


/** A level editor UI element responsible for allowing the user to see all the elements of the active scene. */
class SceneInspector final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Destroy this scene inspector. */
	inline ~SceneInspector() = default;
	/** Construct a scene inspector.
	@param	engine		reference to the engine to use. 
	@param	editor		reference to the level-editor to use. */
	SceneInspector(Engine& engine, LevelEditor_Module& editor) noexcept;


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) noexcept override final;


private:
	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
};

#endif // SCENEINSPECTOR_H