#pragma once
#ifndef SELECTION_GIZMO_H
#define SELECTION_GIZMO_H

#include "Assets/Auto_Model.h"
#include "Assets/Shader.h"
#include "Modules/Editor/Gizmos/Translation.h"
#include "Modules/Editor/Gizmos/Scaling.h"
#include "Modules/Editor/Gizmos/Rotation.h"
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


/** A 3D tool allowing the user to select entities in a level. */
class Mouse_Gizmo {
public:
	// Public (De)Constructors
	/** Destroy this gizmo. */
	~Mouse_Gizmo() noexcept;
	/** Construct this gizmo.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	Mouse_Gizmo(Engine& engine, LevelEditor_Module& editor) noexcept;


	// Public Methods
	/** Tick this gizmo, checking for input and rendering.
	@param	deltaTime		the amount of time since the last frame. */
	void frameTick(const float& deltaTime) noexcept;
	/** Check for user input.
	@param	deltaTime		the amount of time since the last frame. */
	bool checkInput(const float& deltaTime) noexcept;
	/** Render this gizmo.
	@param	deltaTime		the amount of time since the last frame. */
	void render(const float& deltaTime) noexcept;
	/** Apply a specific transform.
	@param	transform		the new transform to use. */
	void setTransform(const Transform& transform) noexcept;
	/** Retrieve this gizmo's transform.
	@return					the transform used by this gizmo. */
	Transform getSelectionTransform() const noexcept;
	/** Retrieve this gizmo's spawn point transform.
	@return					the transform used for spawn points. */
	Transform getSpawnTransform() const noexcept;
	/** Set a specific set of entities as the selection, moving the gizmo to their center.
	@param	entityHandles	the new set of selected entity handles to use. */
	void setSelection(const std::vector<EntityHandle>& entityHandles) noexcept;
	/** Retrieve the current set of selected entities.
	@return					the active set of selected entity handles. */
	std::vector<EntityHandle>& getSelection() noexcept;


private:
	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
	Transform m_selectionTransform, m_spawnTransform;
	std::vector<EntityHandle> m_selection;
	std::shared_ptr<ecsBaseSystem> m_pickerSystem;
	unsigned int m_inputMode = 0;
	Translation_Gizmo m_translationGizmo;
	Scaling_Gizmo m_scalingGizmo;
	Rotation_Gizmo m_rotationGizmo;
	Shared_Auto_Model m_spawnModel;
	Shared_Shader m_spawnShader;
	IndirectDraw<1> m_spawnIndirect;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SELECTION_GIZMO_H
