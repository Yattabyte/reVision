#pragma once
#ifndef SELECTION_GIZMO_H
#define SELECTION_GIZMO_H

#include "Modules/World/ECS/ecsComponent.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Utilities/Transform.h"
#include <memory>
#include <vector>


// Forward Declarations
class Engine;
class LevelEditor_Module;
class Translation_Gizmo;
class Scaling_Gizmo;
class Rotation_Gizmo;


/** A 3D tool allowing the user to select entities in a level. */
class Selection_Gizmo {
public:
	// Public (de)Constructors
	/** Destroy this gizmo. */
	inline ~Selection_Gizmo() = default;
	/** Construct this gizmo.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	Selection_Gizmo(Engine* engine, LevelEditor_Module* editor);


	// Public Methods
	/** Tick this gizmo, checking for input and rendering.
	@param	deltaTime	the amount of time since the last frame. */
	void frameTick(const float& deltaTime);
	/** Check for user input.
	@param	deltaTime	the amount of time since the last frame. */
	bool checkInput(const float& deltaTime);
	/** Render this gizmo.
	@param	deltaTime	the amount of time since the last frame. */
	void render(const float& deltaTime);
	/** Apply a specific transform.
	@param	transform	the new transform to use. */
	void setTransform(const Transform& transform);
	/** Retrieve this gizmo's transform.
	@return				the transform used by this gizmo. */
	Transform getTransform() const;
	/** Set a specific set of entities as the selection, moving the gizmo to their center.
	@param	entities	the new set of selected entities to use. */
	void setSelection(const std::vector<ecsEntity*>& entities);
	/** Retrieve the current set of selected entities.
	@return				the active set of selected entities. */
	std::vector<ecsEntity*>& getSelection();
	

private:
	// Private Methods
	/** Check for mouse input.
	@param	deltaTime	the amount of time since the last frame. */
	bool checkMouseInput(const float& deltaTime);


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	bool m_clicked = false;
	Transform m_transform = glm::vec3(0.0f);
	std::vector<ecsEntity*> m_selection;
	std::shared_ptr<BaseECSSystem> m_pickerSystem;
	unsigned int m_inputMode = 0;
	std::shared_ptr<Translation_Gizmo> m_translationGizmo;
	std::shared_ptr<Scaling_Gizmo> m_scalingGizmo;
	std::shared_ptr<Rotation_Gizmo> m_rotationGizmo;
};

#endif // SELECTION_GIZMO_H