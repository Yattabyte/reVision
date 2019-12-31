#pragma once
#ifndef SELECTION_GIZMO_H
#define SELECTION_GIZMO_H

#include "Assets/Auto_Model.h"
#include "Assets/Shader.h"
#include "Modules/Editor/Gizmos/Translation.h"
#include "Modules/Editor/Gizmos/Scaling.h"
#include "Modules/Editor/Gizmos/Rotation.h"
#include "Modules/Editor/Systems/MousePicker_System.h"
#include "Modules/ECS/ecsEntity.h"
#include "Modules/ECS/ecsSystem.h"
#include "Utilities/Transform.h"
#include "Utilities/GL/IndirectDraw.h"
#include <vector>


// Forward Declarations
class Engine;
class LevelEditor_Module;

/** A 3D tool allowing the user to select entities in a level. */
class Mouse_Gizmo {
public:
	// Public (De)Constructors
	/** Destroy this gizmo. */
	~Mouse_Gizmo();
	/** Construct this gizmo.
	@param	engine		reference to the engine to use.
	@param	editor		reference to the level-editor to use. */
	Mouse_Gizmo(Engine& engine, LevelEditor_Module& editor);


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
	void setTransform(const Transform& transform) noexcept;
	/** Retrieve this gizmo's transform.
	@return					the transform used by this gizmo. */
	Transform getSelectionTransform() const noexcept;
	/** Retrieve this gizmo's spawn point transform.
	@return					the transform used for spawn points. */
	Transform getSpawnTransform() const noexcept;
	/** Set a specific set of entities as the selection, moving the gizmo to their center.
	@param	entityHandles	the new set of selected entity handles to use. */
	void setSelection(const std::vector<EntityHandle>& entityHandles);
	/** Retrieve the current set of selected entities.
	@return					the active set of selected entity handles. */
	std::vector<EntityHandle>& getSelection() noexcept;


private:
	// Private but deleted
	/** Disallow default constructor. */
	inline Mouse_Gizmo() noexcept = delete;
	/** Disallow move constructor. */
	inline Mouse_Gizmo(Mouse_Gizmo&&) noexcept = delete;
	/** Disallow copy constructor. */
	inline Mouse_Gizmo(const Mouse_Gizmo&) noexcept = delete;
	/** Disallow move assignment. */
	inline Mouse_Gizmo& operator =(Mouse_Gizmo&&) noexcept = delete;
	/** Disallow copy assignment. */
	inline Mouse_Gizmo& operator =(const Mouse_Gizmo&) noexcept = delete;


	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
	Transform m_selectionTransform, m_spawnTransform;
	std::vector<EntityHandle> m_selection;
	MousePicker_System m_pickerSystem;
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