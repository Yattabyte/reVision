#pragma once
#ifndef EDITOR_MODULE_H
#define EDITOR_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsWorld.h"
#include "Modules/Editor/Gizmos/Mouse.h"
#include "Modules/Editor/UI/Editor_Interface.h"
#include "Assets/Auto_Model.h"
#include "Assets/Shader.h"
#include "Utilities/Transform.h"
#include "Utilities/GL/IndirectDraw.h"
#include <deque>


// Forward Declarations
class Editor_Interface;
struct Editor_Command;

/** A level editor module. */
class LevelEditor_Module final : public Engine_Module {
public:
	// Public (De)Constructors
	/** Destroy this game module. */
	inline ~LevelEditor_Module() noexcept = default;
	/** Construct a game module.
	@param	engine		reference to the engine to use. */
	explicit LevelEditor_Module(Engine& engine) noexcept;


	// Public Interface Implementation
	void initialize() noexcept final;
	void deinitialize() noexcept final;


	// Public Methods
	/** Tick this module by a specific amount of delta time.
	@param	deltaTime		the amount of time since last frame. */
	void frameTick(const float& deltaTime) noexcept;
	/** Display the level editor. */
	void showEditor() noexcept;
	/** Close the level editor, returning to the main menu. */
	void exit() noexcept;
	/** Check if the editor has any unsaved changes.
	@return					true if the level has unsaved changes, false otherwise. */
	bool hasUnsavedChanges() const noexcept;
	/** Retrieve a reference to the currently active ecsWorld in the editor.
	@return					reference to the currently active ecsWorld. */
	ecsWorld& getWorld() noexcept;
	/** Retrieve the currently active map's file name.
	@return					current map's file name. */
	std::string getMapName() const noexcept;
	/** Retrieve a list of recently opened levels.
	@return					list of recently opened levels. */
	std::deque<std::string> getRecentLevels() const noexcept;
	/** Close the current level, starting a new one. */
	void newLevel() noexcept;
	/** Open a specific level with the file name specified.
	@param	name			the level name to open. */
	void openLevel(const std::string& name) noexcept;
	/** Display the 'open level' dialogue for choosing a level.	*/
	void openLevelDialogue() noexcept;
	/** Save the level with a specific name.
	@param	name			the level name to save. */
	void saveLevel(const std::string& name) noexcept;
	/** Save and overwrite the currently active level. */
	void saveLevel() noexcept;
	/** Display the 'save level' dialogue for choosing a level to save. */
	void saveLevelDialogue() noexcept;
	/** Display the 'settings' dialogue for the level editor. */
	void openSettingsDialogue() noexcept;
	/** Retrieve if we have any undo-able actions. 
	@return					true if able to undo, false otherwise. */
	bool canUndo() const noexcept;
	/** Retrieve if we have any redo-able actions. 
	@return					true if able to redo, false otherwise. */
	bool canRedo() const noexcept;
	/** Undo the previous action in the undo stack. */
	void undo() noexcept;
	/** Redo the previously undone action in the redo stack. */
	void redo() noexcept;
	/** De-select all level entities. */
	void clearSelection() noexcept;
	/** Select all level entities. */
	void selectAll() noexcept;
	/** Select a specific set of entities.
	@param	handles			the new set of entity handles to make selected. */
	void setSelection(const std::vector<EntityHandle>& handles) noexcept;
	/** Retrieve the set of selected entities.
	@return					all selected entity handles. */
	std::vector<EntityHandle>& getSelection() noexcept;
	/** Parent all selected entities into the first one in the set. */
	void mergeSelection() noexcept;
	/** Create a new entity and parent all selected entities into it. */
	void groupSelection() noexcept;
	/** Un-parent all selected entities from their common parent. */
	void ungroupSelection() noexcept;
	/** Save the current set of selected entities into a prefab. */
	void makePrefab() noexcept;
	/** Delete the current selection and place it on the clipboard. */
	void cutSelection() noexcept;
	/** Copy the current selection to the clipboard. */
	void copySelection() noexcept;
	/** Paste whatever is on the clipboard into the level. */
	void paste() noexcept;
	/** Delete the currently selected entities from the level. */
	void deleteSelection() noexcept;
	/** Add a new blank component to an entity given its handle and component name alone.
	@param	entityHandle	handle to the entity the component will be added to.
	@param	name			the component class name. */
	void makeComponent(const EntityHandle& entityHandle, const char* name) noexcept;
	/** Delete the component given its entity and component ID.
	@param	entityHandle	handle to the entity the component belongs to.
	@param	componentID		the runtime ID for this component. */
	void deleteComponent(const EntityHandle& entityHandle, const int& componentID) noexcept;
	/** Spawn a serialized entity into the level.
	@param	entityData		the serialized entity data.
	@param	parent			optional parent's handle. */
	void addEntity(const std::vector<char>& entityData, const EntityHandle& parentUUID = EntityHandle()) noexcept;
	/** Bind the editor's FBO to the currently active GL context, for rendering. */
	void bindFBO() noexcept;
	/** Bind the editor's screen texture to the currently active GL context.
	@param	offset			specific shader texture unit. */
	void bindTexture(const GLuint& offset = 0) noexcept;
	/** Specify a new transform for the gizmo's.
	@param	transform		the specific transform to use. */
	void setGizmoTransform(const Transform& transform) noexcept;
	/** Retrieve the transform from the editor gizmo's.
	@return					the current transform from where the gizmo is. */
	Transform getGizmoTransform() const noexcept;
	/** Retrieve the spawn point transform from the mouse gizmo.
	@return					the current transform from where the spawn point is. */
	Transform getSpawnTransform() const noexcept;
	/** Retrieve the editor's camera position.
	@return					the current camera's position. */
	glm::vec3 getCameraPosition() const noexcept;
	/** Try to add an entity to the selection, removing it if it's already present.
	@param	entityHandle	the entity to attempt to (de)select. */
	void toggleAddToSelection(const EntityHandle& entityHandle) noexcept;
	/** Retrieve if we have any data on the clipboard.
	@return					true if clipboard data present, false otherwise. */
	bool hasCopy() const noexcept;
	/** Make the scene inspector visible. */
	void openSceneInspector() noexcept;
	/** Make the entity inspector visible. */
	void openEntityInspector() noexcept;
	/** Make the prefabs window visible. */
	void openPrefabs() noexcept;
	/** Perform an action following the Command design pattern, executing it and appending it to an undo list.
	@param	command			the command to execute and store. */
	void doReversableAction(const std::shared_ptr<Editor_Command>& command) noexcept;


private:
	// Private and deleted
	/** Disallow module move constructor. */
	inline LevelEditor_Module(LevelEditor_Module&&) noexcept = delete;
	/** Disallow module copy constructor. */
	inline LevelEditor_Module(const LevelEditor_Module&) noexcept = delete;
	/** Disallow module move assignment. */
	inline const LevelEditor_Module& operator =(LevelEditor_Module&&) noexcept = delete;
	/** Disallow module copy assignment. */
	inline const LevelEditor_Module& operator =(const LevelEditor_Module&) noexcept = delete;


	// Private Methods
	/** Add a level name to the 'recent maps' list.
	@param	name			a level name to add to the recent maps list. */
	void addToRecentList(const std::string& name) noexcept;
	/** Populate the 'recent maps' list from disk. */
	void populateRecentList() noexcept;
	/** Save the level with a specific name.
	@param	name			the level name to save. */
	void saveLevel_Internal(const std::string& name) noexcept;



	// Private Attributes
	bool m_active = false, m_unsavedChanges = false;
	float m_autoSaveCounter = 0.0f, m_autosaveInterval = 60.0f;
	Shared_Auto_Model m_shapeQuad;
	Shared_Shader m_shader;
	IndirectDraw<1> m_indirectQuad;
	std::string m_currentLevelName = "My Map.bmap";
	ecsWorld m_world;
	GLuint m_fboID = 0, m_texID = 0, m_depthID = 0;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::vector<char> m_copiedData;
	std::deque<std::shared_ptr<Editor_Command>> m_undoStack, m_redoStack;
	int m_maxUndo = 500;
	std::deque<std::string> m_recentLevels;
	Editor_Interface m_editorInterface;
	Mouse_Gizmo m_mouseGizmo;
	std::shared_ptr<ecsBaseSystem> m_systemSelClearer, m_systemOutline;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

/** A command used by the level editor. Follows the Command Design Pattern.
To be sub-classed where needed, typically within the scope of a specialized function. */
struct Editor_Command {
	// Public Interface
	inline virtual ~Editor_Command() noexcept = default;
	/** Perform the command. */
	virtual void execute() noexcept = 0;
	/** Perform the reverse, undo the command. */
	virtual void undo() noexcept = 0;
	/** Join into this command the data found in another newer command.
	@param	newerCommand	the newer of the two commands, to take data from.
	@return					true if this command supports & successfully joined with a newer command, false otherwise. */
	virtual bool join(Editor_Command* newerCommand) noexcept;
};

#endif // EDITOR_MODULE_H