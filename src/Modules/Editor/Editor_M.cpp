#include "Modules/Editor/Editor_M.h"
#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/Editor/UI/CameraController.h"
#include "Modules/Editor/UI/RotationIndicator.h"
#include "Modules/Editor/UI/TitleBar.h"
#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/UI/Inspector.h"
#include "Modules/Editor/UI/RecoverDialogue.h"
#include "Modules/Editor/UI/OpenDialogue.h"
#include "Modules/Editor/UI/SaveDialogue.h"
#include "Modules/Editor/UI/SettingsDialogue.h"
#include "Modules/Editor/UI/UnsavedChangesDialogue.h"
#include "Modules/Editor/Gizmos/Mouse.h"
#include "Modules/Editor/Systems/ClearSelection_System.h"
#include "Modules/Editor/Systems/Wireframe_System.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Modules/World/ECS/components.h"
#include "Engine.h"
#include <filesystem>


void LevelEditor_Module::initialize(Engine* engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: Level Edtior...");

	// Update indicator
	*m_aliveIndicator = true;

	// UI
	m_editorInterface = std::make_shared<Editor_Interface>(engine, this);

	// Gizmos
	m_mouseGizmo = std::make_shared<Mouse_Gizmo>(engine, this);

	// Systems
	m_selectionClearer = std::make_shared<ClearSelection_System>(engine);
	m_systems.makeSystem<Wireframe_System>(engine, this);

	// Preferences
	auto& preferences = engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) {
		m_renderSize.x = (int)f;
		glTextureImage2DEXT(m_texID, GL_TEXTURE_2D, 0, GL_RGBA16F, m_renderSize.x, m_renderSize.y, 0, GL_RGBA, GL_FLOAT, 0);
		glTextureImage2DEXT(m_depthID, GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, m_renderSize.x, m_renderSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) {
		m_renderSize.y = (int)f;
		glTextureImage2DEXT(m_texID, GL_TEXTURE_2D, 0, GL_RGBA16F, m_renderSize.x, m_renderSize.y, 0, GL_RGBA, GL_FLOAT, 0);
		glTextureImage2DEXT(m_depthID, GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, m_renderSize.x, m_renderSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	});
	preferences.getOrSetValue(PreferenceState::E_AUTOSAVE_INTERVAL, m_autosaveInterval);
	preferences.addCallback(PreferenceState::E_AUTOSAVE_INTERVAL, m_aliveIndicator, [&](const float& f) {
		m_autosaveInterval = f;
	});
	float undoStacksize = 500.0f;
	preferences.getOrSetValue(PreferenceState::E_UNDO_STACKSIZE, undoStacksize);
	m_maxUndo = int(undoStacksize);
	preferences.addCallback(PreferenceState::E_UNDO_STACKSIZE, m_aliveIndicator, [&](const float& f) {
		m_maxUndo = int(f);
	});

	// GL structures
	glCreateFramebuffers(1, &m_fboID);
	glCreateTextures(GL_TEXTURE_2D, 1, &m_texID);
	glTextureParameteri(m_texID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_texID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_texID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureImage2DEXT(m_texID, GL_TEXTURE_2D, 0, GL_RGBA16F, m_renderSize.x, m_renderSize.y, 0, GL_RGBA, GL_FLOAT, 0);
	glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_texID, 0);
	glCreateTextures(GL_TEXTURE_2D, 1, &m_depthID);
	glTextureParameteri(m_depthID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_depthID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_depthID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_depthID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureImage2DEXT(m_depthID, GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, m_renderSize.x, m_renderSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glNamedFramebufferTexture(m_fboID, GL_DEPTH_ATTACHMENT, m_depthID, 0);
	glNamedFramebufferDrawBuffer(m_fboID, GL_COLOR_ATTACHMENT0);
}

void LevelEditor_Module::deinitialize()
{
	m_engine->getManager_Messages().statement("Unloading Module: Level Edtior...");

	// Update indicator
	*m_aliveIndicator = false;

	glDeleteFramebuffers(1, &m_fboID);
	glDeleteTextures(1, &m_texID);
	glDeleteTextures(1, &m_depthID);
}

void LevelEditor_Module::frameTick(const float& deltaTime)
{
	if (m_active) {
		constexpr GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		constexpr GLfloat clearDepth = 1.0f;
		glEnable(GL_DEPTH_TEST);
		glDepthMask(true);
		glClearNamedFramebufferfv(m_fboID, GL_COLOR, 0, clearColor);
		glClearNamedFramebufferfv(m_fboID, GL_DEPTH, 0, &clearDepth);

		// Tick all tools this frame
		m_mouseGizmo->frameTick(deltaTime);
		m_engine->getModule_World().updateSystems(m_systems, deltaTime);

		glDepthMask(false);
		glDisable(GL_DEPTH_TEST);

		// Auto-save
		if (hasUnsavedChanges()) {
			m_autoSaveCounter += deltaTime;
			if (m_autoSaveCounter > m_autosaveInterval) {
				m_autoSaveCounter -= m_autosaveInterval;
				m_engine->getManager_Messages().statement("Autosaving Map...");
				std::filesystem::path currentPath(m_currentLevelName);
				currentPath.replace_extension(".autosave");
				m_engine->getModule_World().saveWorld(currentPath.string());
			}
		}
		else
			m_autoSaveCounter = 0.0f;
	}
}

void LevelEditor_Module::setGizmoTransform(const Transform& transform)
{
	m_mouseGizmo->setTransform(transform);
}

Transform LevelEditor_Module::getGizmoTransform() const
{
	return m_mouseGizmo->getSelectionTransform();
}

Transform LevelEditor_Module::getSpawnTransform() const
{
	return m_mouseGizmo->getSpawnTransform();
}

const glm::vec3& LevelEditor_Module::getCameraPosition() const
{
	return m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition;
}

void LevelEditor_Module::toggleAddToSelection(const ecsHandle& entityHandle)
{
	auto selectionCopy = m_mouseGizmo->getSelection();

	// If the entity is already selected, deselect it
	if (std::find(selectionCopy.cbegin(), selectionCopy.cend(), entityHandle) != selectionCopy.cend())
		std::remove(selectionCopy.begin(), selectionCopy.end(), entityHandle);
	else
		selectionCopy.push_back(entityHandle);

	// Ensure our gizmos stay in sync
	setSelection(selectionCopy);
}

bool LevelEditor_Module::hasCopy() const
{
	return m_copiedData.size() ? true : false;
}

void LevelEditor_Module::showEditor()
{
	m_active = true;
	m_engine->getModule_UI().clear();
	m_engine->getModule_UI().setRootElement(m_editorInterface);
	newLevel();

	for (const auto& item : std::filesystem::recursive_directory_iterator(Engine::Get_Current_Dir() + "\\Maps\\")) {
		const auto& path = item.path();
		if (path.has_extension() && path.extension().string() == ".autosave") {
			m_editorInterface->m_uiRecoverDialogue->startDialogue(path);
			break;
		}
	}
}

void LevelEditor_Module::exit()
{
	m_editorInterface->m_uiUnsavedDialogue->tryPrompt([&]() { 
		m_engine->goToMainMenu();
		m_currentLevelName = "My Map.bmap";
		m_unsavedChanges = false;
		m_undoStack = {};
		m_redoStack = {};
		m_active = false;
	});
}

bool LevelEditor_Module::hasUnsavedChanges() const
{
	return m_unsavedChanges;
}

std::string LevelEditor_Module::getMapName() const
{
	return m_currentLevelName;
}

void LevelEditor_Module::newLevel()
{
	m_editorInterface->m_uiUnsavedDialogue->tryPrompt([&]() {
		m_engine->getModule_World().unloadWorld();
		m_currentLevelName = "My Map.bmap";

		// Starting new level, changes will be discarded
		m_unsavedChanges = false;
		m_undoStack = {};
		m_redoStack = {}; 
	});
}

void LevelEditor_Module::openLevel(const std::string& name)
{
	m_engine->getModule_World().loadWorld(name);
	m_currentLevelName = name;

	// Starting new level, changes will be discarded
	m_unsavedChanges = false;
	m_undoStack = {};
	m_redoStack = {};
}

void LevelEditor_Module::openLevelDialogue()
{
	m_editorInterface->m_uiUnsavedDialogue->tryPrompt([&]() {
		m_editorInterface->m_uiOpenDialogue->startDialogue();
	});
}

void LevelEditor_Module::saveLevel(const std::string& name)
{
	// Make sure the level has a valid name, otherwise open the naming dialogue
	m_currentLevelName = name;
	if (name == "")
		saveLevelDialogue();
	else {
		std::filesystem::path currentPath(m_currentLevelName);
		currentPath.replace_extension(".bmap");
		m_currentLevelName = currentPath.string();
		m_engine->getModule_World().saveWorld(m_currentLevelName);

		// Delete Autosaves
		currentPath.replace_extension(".autosave");
		currentPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\" + currentPath.string());
		if (std::filesystem::exists(currentPath) && !std::filesystem::is_directory(currentPath))
			std::filesystem::remove(currentPath);

		// Self Explanitory
		m_unsavedChanges = false;
	}
}

void LevelEditor_Module::saveLevel()
{
	saveLevel(m_currentLevelName);
}

void LevelEditor_Module::saveLevelDialogue()
{
	m_editorInterface->m_uiSaveDialogue->startDialogue();
}

void LevelEditor_Module::openSettingsDialogue()
{
	m_editorInterface->m_uiSettingsDialogue->startDialogue();
}

bool LevelEditor_Module::canUndo() const
{
	return m_undoStack.size();
}

bool LevelEditor_Module::canRedo() const
{
	return m_redoStack.size();
}

void LevelEditor_Module::undo()
{
	if (m_undoStack.size()) {
		// Undo the the last action
		m_undoStack.front()->undo();

		// Move the action onto the redo stack
		m_redoStack.push_front(m_undoStack.front());
		m_undoStack.pop_front();

		// Set unsaved changes all the time
		m_unsavedChanges = true;
	}
}

void LevelEditor_Module::redo()
{
	if (m_redoStack.size()) {
		// Redo the last action
		m_redoStack.front()->execute();

		// Push the action onto the undo stack
		m_undoStack.push_front(m_redoStack.front());
		m_redoStack.pop_front();

		// Set unsaved changes unless we have no more redo actions
		m_unsavedChanges = bool(m_redoStack.size() != 0ull);
	}
}

void LevelEditor_Module::doReversableAction(const std::shared_ptr<Editor_Command>& command)
{
	// Clear the redo stack
	m_redoStack = {};

	// Perform the desired action
	command->execute();

	// Add action to the undo stack
	m_undoStack.push_front(command);
	while (m_undoStack.size() > m_maxUndo)
		m_undoStack.pop_back();
	m_unsavedChanges = true;
}

void LevelEditor_Module::clearSelection()
{
	struct Clear_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::vector<ecsHandle> m_uuids_old;
		Clear_Selection_Command(Engine* engine, LevelEditor_Module* editor)
			: m_engine(engine), m_editor(editor), m_uuids_old(m_editor->getSelection()) {}
		void switchSelection(const std::vector<ecsHandle>& uuids) {
			// Remove all selection components from world
			auto& world = m_engine->getModule_World();
			world.updateSystem(m_editor->m_selectionClearer.get(), 0.0f);
			m_editor->m_mouseGizmo->getSelection().clear();

			// Add selection component to new selection
			m_editor->m_mouseGizmo->setSelection(uuids);
			for each (const auto& entityHandle in uuids) {
				const Selected_Component component;
				world.addComponent(entityHandle, &component);
			}

			// Transform gizmo to center of group
			Transform newTransform;
			size_t count(0ull);
			glm::vec3 center(0.0f), scale(0.0f);
			for each (const auto & entityHandle in uuids)
				if (auto * transform = world.getComponent<Transform_Component>(entityHandle)) {
					center += transform->m_localTransform.m_position;
					scale += transform->m_localTransform.m_scale;
					count++;
				}
			center /= count;
			scale /= count;
			newTransform.m_position = center;
			newTransform.m_scale = scale;
			newTransform.update();
			m_editor->m_mouseGizmo->setTransform(newTransform);
		};
		virtual void execute() {
			// Remove all selection components from world
			auto& world = m_engine->getModule_World();
			world.updateSystem(m_editor->m_selectionClearer.get(), 0.0f);
			m_editor->m_mouseGizmo->getSelection().clear();
		}
		virtual void undo() {
			// Remove all selection components from world
			auto& world = m_engine->getModule_World();
			world.updateSystem(m_editor->m_selectionClearer.get(), 0.0f);
			m_editor->m_mouseGizmo->getSelection().clear();

			// Add selection component to new selection
			m_editor->m_mouseGizmo->setSelection(m_uuids_old);
			for each (const auto& entityHandle in m_uuids_old) {
				const Selected_Component component;
				world.addComponent(entityHandle, &component);
			}

			// Transform gizmo to center of group
			Transform newTransform;
			size_t count(0ull);
			glm::vec3 center(0.0f), scale(0.0f);
			for each (const auto & entityHandle in m_uuids_old)
				if (auto * transform = world.getComponent<Transform_Component>(entityHandle)) {
					center += transform->m_localTransform.m_position;
					scale += transform->m_localTransform.m_scale;
					count++;
				}
			center /= count;
			scale /= count;
			newTransform.m_position = center;
			newTransform.m_scale = scale;
			newTransform.update();
			m_editor->m_mouseGizmo->setTransform(newTransform);
		}
	};

	if (getSelection().size())
		doReversableAction(std::make_shared<Clear_Selection_Command>(m_engine, this));
}

void LevelEditor_Module::selectAll()
{
	setSelection(m_engine->getModule_World().getEntityHandles());
}

void LevelEditor_Module::setSelection(const std::vector<ecsHandle>& handles)
{
	struct Set_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::vector<ecsHandle> m_uuids_new, m_uuids_old;
		Set_Selection_Command(Engine* engine, LevelEditor_Module* editor, const std::vector<ecsHandle>& newSelection)
			: m_engine(engine), m_editor(editor), m_uuids_new(newSelection), m_uuids_old(m_editor->getSelection()) {}
		void switchSelection(const std::vector<ecsHandle>& uuids) {
			// Remove all selection components from world
			auto& world = m_engine->getModule_World();
			world.updateSystem(m_editor->m_selectionClearer.get(), 0.0f);
			m_editor->m_mouseGizmo->getSelection().clear();

			// Add selection component to new selection
			m_editor->m_mouseGizmo->setSelection(uuids);
			for each (const auto& entityHandle in uuids) {
				const Selected_Component component;
				world.addComponent(entityHandle, &component);
			}

			// Transform gizmo to center of group
			Transform newTransform;
			size_t count(0ull);
			glm::vec3 center(0.0f), scale(0.0f);
			for each (const auto & entityHandle in uuids)
				if (auto * transform = world.getComponent<Transform_Component>(entityHandle)) {
					center += transform->m_localTransform.m_position;
					scale += transform->m_localTransform.m_scale;
					count++;
				}
			center /= count;
			scale /= count;
			newTransform.m_position = center;
			newTransform.m_scale = scale;
			newTransform.update();
			m_editor->m_mouseGizmo->setTransform(newTransform);
		};
		virtual void execute() {
			switchSelection(m_uuids_new);
		}
		virtual void undo() {
			switchSelection(m_uuids_old);
		}
	};

	if (handles.size())
		doReversableAction(std::make_shared<Set_Selection_Command>(m_engine, this, handles));
}

const std::vector<ecsHandle>& LevelEditor_Module::getSelection() const
{
	return m_mouseGizmo->getSelection();
}

void LevelEditor_Module::mergeSelection()
{
	struct Merge_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::vector<ecsHandle> m_uuids;
		Merge_Selection_Command(Engine* engine, LevelEditor_Module* editor)
			: m_engine(engine), m_editor(editor), m_uuids(m_editor->getSelection()) {}
		virtual void execute() {
			auto& world = m_engine->getModule_World();
			// Find the root element
			const auto& root = m_uuids[0];
			if (root.isValid()) {
				// Parent remaining entities in the selection to the root
				for (size_t x = 1ull, selSize = m_uuids.size(); x < selSize; ++x) 
					if (const auto & entityHandle = m_uuids[x])
						world.parentEntity(root, entityHandle);				
				m_editor->m_mouseGizmo->getSelection() = { root };
			}
		}
		virtual void undo() {
			auto& world = m_engine->getModule_World();
			// Find the root element
			if (auto * root = world.getEntity(m_uuids[0])) {
				// Unparent remaining entities from the root
				for (size_t x = 1ull, selSize = m_uuids.size(); x < selSize; ++x) 
					if (const auto & entityHandle = m_uuids[x])
						world.unparentEntity(entityHandle);				
			}
		}
	};

	auto& selection = m_mouseGizmo->getSelection();
	if (m_mouseGizmo->getSelection().size())
		doReversableAction(std::make_shared<Merge_Selection_Command>(m_engine, this));
}

void LevelEditor_Module::groupSelection()
{
	struct Group_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::vector<ecsHandle> m_uuids;
		ecsHandle m_rootUUID;
		Group_Selection_Command(Engine* engine, LevelEditor_Module* editor)
			: m_engine(engine), m_editor(editor), m_uuids(m_editor->getSelection()) {}
		virtual void execute() {
			// Determine a new central transform for the whole group
			auto& world = m_engine->getModule_World();
			Transform_Component rootTransform;
			size_t posCount(0ull);
			for each (const auto& entityHandle in m_uuids)
				if (const auto & transform = world.getComponent<Transform_Component>(entityHandle)) {
					rootTransform.m_localTransform.m_position += transform->m_localTransform.m_position;
					posCount++;
				}
			rootTransform.m_localTransform.m_position /= float(posCount);
			rootTransform.m_localTransform.update();
			rootTransform.m_worldTransform = rootTransform.m_localTransform;

			// Make a new root entity for the selection
			BaseECSComponent* entityComponents[] = { &rootTransform };
			int types[] = { Transform_Component::ID };
			auto root = world.makeEntity(entityComponents, types, 1ull, "Group", m_rootUUID);
			if (m_rootUUID == ecsHandle())
				m_rootUUID = root;

			// Offset children by new center position
			for each (auto & uuid in m_uuids)
				world.parentEntity(root, uuid);
		}
		virtual void undo() {
			auto& world = m_engine->getModule_World();
			auto& selection = m_editor->m_mouseGizmo->getSelection();
			selection.clear();
			if (m_rootUUID != ecsHandle()) {
				for each (const auto & child in world.getEntityHandles(m_rootUUID)) {
					world.unparentEntity(child);
					selection.push_back(child);
				}
				world.removeEntity(m_rootUUID);
			}
		}
	};

	if (m_mouseGizmo->getSelection().size())
		doReversableAction(std::make_shared<Group_Selection_Command>(m_engine, this));
}

void LevelEditor_Module::ungroupSelection()
{
	struct Ungroup_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::vector<ecsHandle> m_uuids;
		std::vector<std::vector<ecsHandle>> m_children;
		Ungroup_Selection_Command(Engine* engine, LevelEditor_Module* editor)
			: m_engine(engine), m_editor(editor), m_uuids(m_editor->getSelection()) {
			auto& world = m_engine->getModule_World();
			for each (const auto & entityHandle in m_uuids) {
				std::vector<ecsHandle> childrenUUIDS;
				for each (const auto & childHandle in world.getEntityHandles(entityHandle))
					childrenUUIDS.push_back(childHandle);
				m_children.push_back(childrenUUIDS);
			}
		}
		virtual void execute() {
			auto& world = m_engine->getModule_World();
			for each (const auto & entityHandle in m_uuids)
				for each (const auto & childHandle in world.getEntityHandles(entityHandle))
					world.unparentEntity(childHandle);
		}
		virtual void undo() {
			auto& world = m_engine->getModule_World();
			size_t childIndex(0ull);
			for each (const auto & enityUUID in m_uuids)
				for each (const auto & childUUID in m_children[childIndex++]) 
					world.parentEntity(enityUUID, childUUID);
		}
	};

	if (m_mouseGizmo->getSelection().size())
		doReversableAction(std::make_shared<Ungroup_Selection_Command>(m_engine, this));
}

void LevelEditor_Module::makePrefab()
{
	m_editorInterface->m_uiPrefabs->makePrefab(getSelection());
}

void LevelEditor_Module::cutSelection()
{
	copySelection();
	deleteSelection();
}

void LevelEditor_Module::copySelection()
{
	m_copiedData.clear();
	auto& world = m_engine->getModule_World();
	for each (const auto & entityHandle in getSelection()) {
		const auto entData = world.serializeEntity(entityHandle);
		m_copiedData.insert(m_copiedData.end(), entData.begin(), entData.end());
	}
}

void LevelEditor_Module::paste()
{
	if (m_copiedData.size())
		addEntity(m_copiedData);
}

void LevelEditor_Module::deleteSelection()
{
	struct Delete_Selection_Command : Editor_Command {
		Engine* m_engine;
		const std::vector<char> m_data;
		const std::vector<ecsHandle> m_uuids;
		Delete_Selection_Command(Engine* engine, const std::vector<ecsHandle>& selection)
			: m_engine(engine), m_data(m_engine->getModule_World().serializeEntities(selection)), m_uuids(selection) {}
		virtual void execute() {
			auto& world = m_engine->getModule_World();
			for each (const auto& entityHandle in m_uuids)
				world.removeEntity(entityHandle);
		}
		virtual void undo() {
			auto& world = m_engine->getModule_World();
			size_t dataRead(0ull), uuidIndex(0ull);
			while (dataRead < m_data.size())
				world.deserializeEntity(m_data.data(), m_data.size(), dataRead);			
		}
	};

	auto& selection = m_mouseGizmo->getSelection();
	if (selection.size())
		doReversableAction(std::make_shared<Delete_Selection_Command>(m_engine, selection));
}

void LevelEditor_Module::moveSelection(const glm::vec3& newPosition)
{
	struct Move_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const glm::vec3 m_oldPosition, m_newPosition;
		const std::vector<ecsHandle> m_uuids;
		Move_Selection_Command(Engine* engine, LevelEditor_Module* editor, const glm::vec3& newPosition)
			: m_engine(engine), m_editor(editor), m_oldPosition(m_editor->m_mouseGizmo->getSelectionTransform().m_position), m_newPosition(newPosition), m_uuids(m_editor->getSelection()) {}
		void move(const glm::vec3& position) {
			auto& world = m_engine->getModule_World();
			std::vector<Transform_Component*> transformComponents;
			glm::vec3 center(0.0f);
			for each (const auto& entityHandle in m_uuids)
				if (auto * transform = world.getComponent<Transform_Component>(entityHandle)) {
					transformComponents.push_back(transform);
					center += transform->m_localTransform.m_position;
				}
			center /= transformComponents.size();
			for each (auto * transform in transformComponents) {
				transform->m_localTransform.m_position = (transform->m_localTransform.m_position - center) + position;
				transform->m_localTransform.update();
			}

			auto gizmoTransform = m_editor->m_mouseGizmo->getSelectionTransform();
			gizmoTransform.m_position = position;
			gizmoTransform.update();
			m_editor->setGizmoTransform(gizmoTransform);
		}
		virtual void execute() {
			move(m_newPosition);
		}
		virtual void undo() {
			move(m_oldPosition);
		}
	};

	doReversableAction(std::make_shared<Move_Selection_Command>(m_engine, this, newPosition));
}

void LevelEditor_Module::rotateSelection(const glm::quat& newRotation)
{
	struct Rotate_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const glm::quat m_newRotation;
		const std::vector<ecsHandle> m_uuids;
		Rotate_Selection_Command(Engine* engine, LevelEditor_Module* editor, const glm::quat& newRotation)
			: m_engine(engine), m_editor(editor), m_newRotation(newRotation), m_uuids(m_editor->getSelection()) {}
		void rotate(const glm::quat& rotation) {
			auto& world = m_engine->getModule_World();
			std::vector<Transform_Component*> transformComponents;
			glm::vec3 center(0.0f);
			for each (const auto& entityHandle in m_uuids)
				if (auto * transform = world.getComponent<Transform_Component>(entityHandle)) {
					transformComponents.push_back(transform);
					center += transform->m_localTransform.m_position;
				}
			center /= transformComponents.size();
			for each (auto * transform in transformComponents) {
				const auto delta = transform->m_localTransform.m_position - center;
				auto rotatedDelta = glm::mat4_cast(rotation) * glm::vec4(delta, 1.0f);
				rotatedDelta /= rotatedDelta.w;
				transform->m_localTransform.m_position = glm::vec3(rotatedDelta) + center;
				transform->m_localTransform.m_orientation = rotation * transform->m_localTransform.m_orientation;
				transform->m_localTransform.update();
			}

			auto gizmoTransform = m_editor->m_mouseGizmo->getSelectionTransform();
			gizmoTransform.m_orientation = rotation;
			gizmoTransform.update();
			m_editor->setGizmoTransform(gizmoTransform);
		}
		virtual void execute() {
			rotate(m_newRotation);
		}
		virtual void undo() {
			rotate(glm::inverse(m_newRotation));
		}
	};

	doReversableAction(std::make_shared<Rotate_Selection_Command>(m_engine, this, newRotation));
}

void LevelEditor_Module::scaleSelection(const glm::vec3& newScale)
{
	struct Scale_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const glm::vec3 m_oldScale, m_newScale;
		const std::vector<ecsHandle> m_uuids;
		Scale_Selection_Command(Engine* engine, LevelEditor_Module* editor, const glm::vec3& newRotation)
			: m_engine(engine), m_editor(editor), m_oldScale(m_editor->m_mouseGizmo->getSelectionTransform().m_scale), m_newScale(newRotation), m_uuids(m_editor->getSelection()) {}
		void scale(const glm::vec3& scale) {
			auto& world = m_engine->getModule_World();
			std::vector<Transform_Component*> transformComponents;
			glm::vec3 center(0.0f);
			for each (const auto &entityHandle in m_uuids)
				if (auto * transform = world.getComponent<Transform_Component>(entityHandle)) {
					transformComponents.push_back(transform);
					center += transform->m_localTransform.m_position;
				}
			center /= transformComponents.size();
			for each (auto * transform in transformComponents) {
				const auto delta = transform->m_localTransform.m_position - center;
				transform->m_localTransform.m_position = ((delta / transform->m_localTransform.m_scale) * scale) + center;
				transform->m_localTransform.m_scale = scale;
				transform->m_localTransform.update();
			}
			auto gizmoTransform = m_editor->m_mouseGizmo->getSelectionTransform();
			gizmoTransform.m_scale = scale;
			gizmoTransform.update();
			m_editor->setGizmoTransform(gizmoTransform);
		}
		virtual void execute() {
			scale(m_newScale);
		}
		virtual void undo() {
			scale(m_oldScale);
		}
	};

	doReversableAction(std::make_shared<Scale_Selection_Command>(m_engine, this, newScale));
}

void LevelEditor_Module::addComponent(const ecsHandle& entityHandle, const char* name)
{
	struct Spawn_Component_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const ecsHandle m_entityHandle;
		const char* m_componentName;
		Spawn_Component_Command(Engine* engine, LevelEditor_Module* editor, const ecsHandle& entityHandle, const char* name)
			: m_engine(engine), m_editor(editor), m_entityHandle(entityHandle), m_componentName(name) {}
		virtual void execute() {
			if (const auto & [templateComponent, componentID, componentSize] = BaseECSComponent::findTemplate(m_componentName); templateComponent != nullptr) {
				auto& world = m_engine->getModule_World();
				auto* clone = templateComponent->clone();
				world.addComponent(m_entityHandle, clone);
				delete clone;
			}
		}
		virtual void undo() {
			auto& world = m_engine->getModule_World();
			if (const auto & [templateComponent, componentID, componentSize] = BaseECSComponent::findTemplate(m_componentName); templateComponent != nullptr)
				for each (auto & component in world.getEntity(m_entityHandle)->m_components)
					if (component.first == componentID) {
						world.removeComponent(m_entityHandle, component.first);
						break;
					}
		}
	};

	doReversableAction(std::make_shared<Spawn_Component_Command>(m_engine, this, entityHandle, name));
}

void LevelEditor_Module::deleteComponent(const ecsHandle& entityHandle, const int& componentID)
{
	struct Delete_Component_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const ecsHandle m_entityHandle;
		std::vector<char> m_componentData;
		const int m_componentID;
		Delete_Component_Command(Engine* engine, LevelEditor_Module* editor, const ecsHandle& entityHandle, const int componentID)
			: m_engine(engine), m_editor(editor), m_entityHandle(entityHandle), m_componentID(componentID) {
			auto& world = m_engine->getModule_World();
			if (const auto & component = world.getComponent(m_entityHandle, m_componentID))
				m_componentData = world.serializeComponent(component);
			
		}
		virtual void execute() {
			auto& world = m_engine->getModule_World();
			world.removeComponent(m_entityHandle, m_componentID);
		}
		virtual void undo() {
			auto& world = m_engine->getModule_World();
			if (m_componentData.size()) {
				size_t dataRead(0ull);
				auto copy = world.deserializeComponent(m_componentData.data(), m_componentData.size(), dataRead);
				world.addComponent(m_entityHandle, copy.first);
				delete copy.first;
			}
		}
	};

	doReversableAction(std::make_shared<Delete_Component_Command>(m_engine, this, entityHandle, componentID));
}

void LevelEditor_Module::addEntity(const std::vector<char>& entityData, const ecsHandle& parentUUID)
{
	struct Spawn_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::vector<char> m_data;
		const ecsHandle m_parentUUID;
		const Transform m_cursor;
		std::vector<ecsHandle> m_uuids;
		Spawn_Command(Engine* engine, LevelEditor_Module* editor, const std::vector<char>& data, const ecsHandle& pUUID)
			: m_engine(engine), m_editor(editor), m_data(data), m_parentUUID(pUUID), m_cursor(m_editor->getSpawnTransform()) {}
		virtual void execute() {
			auto& world = m_engine->getModule_World();
			size_t dataRead(0ull), handleCount(0ull);
			glm::vec3 center(0.0f);
			std::vector<Transform_Component*> transformComponents;
			while (dataRead < m_data.size()) {
				// Ensure we have a vector large enough to hold all UUIDs, but maintain previous data
				m_uuids.resize(std::max<size_t>(m_uuids.size(), handleCount + 1ull));
				const auto desiredHandle = m_uuids[handleCount].isValid() ? m_uuids[handleCount] : world.generateUUID();
				const auto& [entityHandle, entity] = world.deserializeEntity(m_data.data(), m_data.size(), dataRead, ecsHandle(), desiredHandle);
				if (entityHandle.isValid() && entity) {
					if (auto * transform = world.getComponent<Transform_Component>(entityHandle)) {
						transformComponents.push_back(transform);
						center += transform->m_localTransform.m_position;
					}
				}
				m_uuids[handleCount++] = entityHandle;
			}
			// Treat entity collection as a group
			// Move the group to world origin, then transform to 3D cursor
			center /= transformComponents.size();
			const auto cursorPos = m_cursor.m_position;
			for each (auto * transform in transformComponents) {
				transform->m_localTransform.m_position = (transform->m_localTransform.m_position - center) + cursorPos;
				transform->m_localTransform.update();
			}
		}
		virtual void undo() {
			auto& world = m_engine->getModule_World();
			for each (const auto& entityHandle in m_uuids)
				world.removeEntity(entityHandle);
		}
	};

	if (entityData.size())
		doReversableAction(std::make_shared<Spawn_Command>(m_engine, this, entityData, parentUUID));
}

void LevelEditor_Module::bindFBO()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
}

void LevelEditor_Module::bindTexture(const GLuint& offset)
{
	glBindTextureUnit(offset, m_texID);
}