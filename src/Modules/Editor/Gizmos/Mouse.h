#pragma once
#ifndef SELECTION_GIZMO_H
#define SELECTION_GIZMO_H

#include "Assets/Auto_Model.h"
#include "Assets/Shader.h"
#include "Modules/ECS/ecsComponent.h"
#include "Modules/ECS/ecsEntity.h"
#include "Modules/ECS/ecsSystem.h"
#include "Utilities/Transform.h"
#include "Utilities/GL/IndirectDraw.h"
#include <memory>
#include <vector>


// Forward Declarations
class Engine;
class LevelEditor_Module;
class Translation_Gizmo;
class Scaling_Gizmo;
class Rotation_Gizmo;


/** A 3D tool allowing the user to select entities in a level. */
class Mouse_Gizmo {
public:
	// Public (de)Constructors
	/** Destroy this gizmo. */
	~Mouse_Gizmo();
	/** Construct this gizmo.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	Mouse_Gizmo(Engine* engine, LevelEditor_Module* editor);


	// Public Methods
	/** Tick this gizmo, checking for input and rendering.
	@param	deltaTime		the amount of time since the last frame. */
	void frameTick(const float& deltaTime);
	/** Check for user input.
	@param	deltaTime		the amount of time since the last frame. */
	bool checkInput(const float& deltaTime);
	/** Render this gizmo.
	@param	deltaTime		the amount of time since the last frame. */
	void render(const float& deltaTime);
	/** Apply a specific transform.
	@param	transform		the new transform to use. */
	void setTransform(const Transform& transform);
	/** Retrieve this gizmo's transform.
	@return					the transform used by this gizmo. */
	Transform getSelectionTransform() const;
	/** Retrieve this gizmo's spawn point transform.
	@return					the transform used for spawn points. */
	Transform getSpawnTransform() const;
	/** Set a specific set of entities as the selection, moving the gizmo to their center.
	@param	entityHandles	the new set of selected entity handles to use. */
	void setSelection(const std::vector<ecsHandle>& entities);
	/** Retrieve the current set of selected entities.
	@return					the active set of selected entity handles. */
	std::vector<ecsHandle>& getSelection();


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	Transform m_selectionTransform, m_spawnTransform;
	std::vector<ecsHandle> m_selection;
	std::shared_ptr<ecsBaseSystem> m_pickerSystem;
	unsigned int m_inputMode = 0;
	std::shared_ptr<Translation_Gizmo> m_translationGizmo;
	std::shared_ptr<Scaling_Gizmo> m_scalingGizmo;
	std::shared_ptr<Rotation_Gizmo> m_rotationGizmo;
	Shared_Auto_Model m_spawnModel;
	Shared_Shader m_spawnShader;
	IndirectDraw m_spawnIndirect;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SELECTION_GIZMO_H