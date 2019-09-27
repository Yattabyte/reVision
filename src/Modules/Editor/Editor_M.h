#pragma once
#ifndef EDITOR_MODULE_H
#define EDITOR_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsEntity.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/UI/UI_M.h"
#include "Utilities/Transform.h"
#include <deque>


// Forward Declarations
class Editor_Interface;
class Mouse_Gizmo;
struct Editor_Command;

/** A level editor module. */
class LevelEditor_Module final : public Engine_Module {
public:
	// Public (de)Constructors
	/** Destroy this game module. */
	inline ~LevelEditor_Module() = default;
	/** Construct a game module. */
	inline LevelEditor_Module() = default;


	// Public Interface Implementation
	virtual void initialize(Engine* engine) override final;
	virtual void deinitialize() override final;
	virtual void frameTick(const float& deltaTime) override final;


	// Public Methods
	/** Display the level editor. */
	void showEditor();
	/** Close the level editor, returning to the main menu. */
	void exit();
	/** Check if the editor has any unsaved changes. */
	bool hasUnsavedChanges() const;
	/** Retrieve the currently active map's file name. */
	std::string getMapName() const;
	/** Close the current level, starting a new one. */
	void newLevel();
	/** Open a specific level with the file name specified.
	@param	name			the level name to open. */
	void openLevel(const std::string& name);
	/** Display the 'open level' dialogue for choosing a level.	*/
	void openLevelDialogue();
	/** Save the level with a specific name.
	@param	name			the level name to save. */
	void saveLevel(const std::string& name);
	/** Save and overwrite the currently active level. */
	void saveLevel();
	/** Display the 'save level' dialogue for choosing a level to save. */
	void saveLevelDialogue();
	/** Display the 'settings' dialogue for the level editor. */
	void openSettingsDialogue();
	/** Retrieve if we have any undo-able actions. */
	bool canUndo() const;
	/** Retrieve if we have any redo-able actions. */
	bool canRedo() const;
	/** Undo the previous action in the undo stack. */
	void undo();
	/** Redo the previously undone action in the redo stack. */
	void redo();
	/** Deselect all level entities. */
	void clearSelection();
	/** Select all level entities. */
	void selectAll();
	/** Select a sepecific set of entities.
	@param	handles			the new set of entity handles to make selected. */
	void setSelection(const std::vector<ecsHandle>& handles);
	/** Retrieve the set of selected entities.
	@return					all selected entity handles. */
	const std::vector<ecsHandle>& getSelection() const;
	/** Parent all selected entities into the first one in the set. */
	void mergeSelection();
	/** Create a new entity and parent all selected entities into it. */
	void groupSelection();
	/** Unparent all selected entities from their common parent. */
	void ungroupSelection();
	/** Save the current set of selected entities into a prefab. */
	void makePrefab();
	/** Delete the current selection and place it on the clipboard. */
	void cutSelection();
	/** Copy the current selection to the clipboard. */
	void copySelection();
	/** Paste whatever is on the clipboard into the level. */
	void paste();
	/** Delete the currently selected entities from the level. */
	void deleteSelection();
	/** Add a new blank component to an entity given its handle and component name alone.
	@param	entityhandle	handle to the entity the component will be added to.
	@param	name			the component class name. */
	void addComponent(const ecsHandle& entityHandle, const char* name);
	/** Delete the component given its entity and component ID.
	@param	entityhandle	handle to the entity the component belongs to.
	@param	componentID		the runtime ID for this component. */
	void deleteComponent(const ecsHandle& entityHandle, const int& componentID);
	/** Spawn a serialized entity into the level.
	@param	entityData		the serialized entity data.
	@param	parent			optional parent's handle. */
	void addEntity(const std::vector<char>& entityData, const ecsHandle& parentUUID = ecsHandle());
	/** Bind the editor's FBO to the currently active GL context, for rendering. */
	void bindFBO();
	/** Bind the editor's screen texture to the currently active GL context.
	@param	offset			specific eshader texture unit. */
	void bindTexture(const GLuint& offset = 0);
	/** Specify a new transform for the gizmos.
	@param	transform		the specific transform to use. */
	void setGizmoTransform(const Transform& transform);
	/** Retrieve the transform from the editor gizmos.
	@return					the current transform from where the gizmo is. */
	Transform getGizmoTransform() const;
	/** Retrieve the spawn point transform from the mouse gizmo.
	@return					the current transform from where the spawn point is. */
	Transform getSpawnTransform() const;
	/** Retrieve the editor's camera position.
	@return					the current camera's position. */
	const glm::vec3& getCameraPosition() const;
	/** Try to add an entity to the selection, removing it if it's already present.
	@param	entityHandle	the entity to attempt to (de)select. */
	void toggleAddToSelection(const ecsHandle& entityHandle);
	/** Retrieve if we have any data on the clipboard.
	@return					true if clipboard data present, false otherwise. */
	bool hasCopy() const;
	/** Make the scene inspector visible. */
	void openSceneInspector();
	/** Make the entity inspector visible. */
	void openEntityInspector();
	/** Make the prefabs window visible. */
	void openPrefabs();
	/** Perform an action following the Command design pattern, executing it and appending it to an undo list.
	@param	command			the command to execute and store. */
	void doReversableAction(const std::shared_ptr<Editor_Command>& command);


private:
	// Private Attributes
	bool m_active = false, m_unsavedChanges = false;
	float m_autoSaveCounter = 0.0f, m_autosaveInterval = 60.0f;
	std::string m_currentLevelName = "My Map.bmap";
	std::shared_ptr<Editor_Interface> m_editorInterface;
	std::shared_ptr<Mouse_Gizmo> m_mouseGizmo;
	std::shared_ptr<ecsBaseSystem> m_systemSelClearer, m_systemOutline;
	GLuint m_fboID = 0, m_texID = 0, m_depthID = 0;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::vector<char> m_copiedData;
	std::deque<std::shared_ptr<Editor_Command>> m_undoStack, m_redoStack;
	int m_maxUndo = 500;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

/** A command used by the level editor. Follows the Command Design Pattern.
To be subclassed where needed, typically within the scope of a specialized function. */
struct Editor_Command {
	// Public Interface
	/** Perform the command. */
	virtual void execute() = 0;
	/** Perform the reverse, undo the command. */
	virtual void undo() = 0;
	/** Join into this command the data found in another newer command.
	@param	newerCommand	the newer of the two commands, to take data from.
	@return					true if this command supports & successfully joined with a newer command, false otherwise. */
	inline virtual bool join(Editor_Command* const newerCommand) { return false; }
};

#endif // EDITOR_MODULE_H