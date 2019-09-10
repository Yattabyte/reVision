#pragma once
#ifndef EDITOR_MODULE_H
#define EDITOR_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/World/ECS/ecsEntity.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/UI/UI_M.h"
#include "Utilities/Transform.h"
#include <stack>


// Forward Declarations
class Editor_Interface;
class Selection_Gizmo;
struct Editor_Command;

/** A level editor module. */
class LevelEditor_Module : public Engine_Module {
public:
	// Public (de)Constructors
	/** Destroy this game module. */
	inline ~LevelEditor_Module() = default;
	/** Construct a game module. */
	inline LevelEditor_Module() = default;


	// Public Interface Implementation
	virtual void initialize(Engine * engine) override;
	virtual void deinitialize() override;
	virtual void frameTick(const float & deltaTime) override;


	// Public Methods
	/** Display the level editor. */
	void showEditor();
	/** Close the level editor, returning to the main menu. */
	void exit();
	/** Close the current level, starting a new one. 
	*@todo	check against dirty bit for 'level has unsaved changes' */
	void newLevel();
	/** Open a specific level with the file name specified.
	@param	name			the level name to open.
	*@todo	check against dirty bit for 'level has unsaved changes' */
	void openLevel(const std::string & name);
	/** Display the 'open level' dialogue for choosing a level.	
	*@todo	check against dirty bit for 'level has unsaved changes' */
	void openLevelDialog();
	/** Save the level with a specific name.
	@param	name			the level name to save. */
	void saveLevel(const std::string & name);
	/** Save and overwrite the currently active level. */
	void saveLevel();
	/** Display the 'save level' dialogue for choosing a level to save.
	*@todo	confirmation check on overwrite. */
	void saveLevelDialog();
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
	/** Translate the current selection to a new position. 
	Supports moving as group. */
	void moveSelection(const glm::vec3& newPosition);
	/** Rotate the current selection to a new orientation.
	Supports rotating as group. */
	void rotateSelection(const glm::quat& newRotation);
	/** Streth the current selection to a new scale.
	Supports scaling as group. */
	void scaleSelection(const glm::vec3& newScale);
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
	void bindTexture(const GLuint & offset = 0);
	/** Specify a new transform for the gizmos.
	@param	transform		the specific transform to use. */
	void setGizmoTransform(const Transform & transform);
	/** Retrieve the transform from the editor gizmos.
	@return					the current transform from where the gizmo is. */
	Transform getGizmoTransform() const;
	/** Retrieve the editor's camera position. 
	@return					the current camera's position. */
	const glm::vec3& getCameraPosition() const;
	/** Try to add an entity to the selection, removing it if it's already present. 
	@param	entityHandle	the entity to attempt to (de)select. */
	void toggleAddToSelection(const ecsHandle& entityHandle);
	/** Retrieve if we have any data on the clipboard. 
	@return					true if clipboard data present, false otherwise. */
	bool hasCopy() const;
	/***/
	void doReversableAction(const std::shared_ptr<Editor_Command>& command);


private:
	// Private Attributes
	std::string m_currentLevelName = "";
	std::shared_ptr<Editor_Interface> m_editorInterface;
	std::shared_ptr<Selection_Gizmo> m_selectionGizmo;
	std::shared_ptr<BaseECSSystem> m_selectionClearer;
	GLuint m_fboID = 0, m_texID = 0, m_depthID = 0;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::vector<char> m_copiedData;
	ECSSystemList m_systems;
	std::stack<std::shared_ptr<Editor_Command>> m_undoStack, m_redoStack;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

struct Editor_Command {
	// Public Interface
	/** Perform the command. */
	virtual void execute() = 0;
	/** Perform the reverse, undo the command. */
	virtual void undo() = 0;
};

#endif // EDITOR_MODULE_H