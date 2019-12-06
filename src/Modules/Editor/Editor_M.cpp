#include "Modules/Editor/Editor_M.h"
#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/UI/RecoverDialogue.h"
#include "Modules/Editor/UI/UnsavedChangesDialogue.h"
#include "Modules/Editor/UI/MissingFileDialogue.h"
#include "Modules/Editor/Gizmos/Mouse.h"
#include "Modules/Editor/Systems/ClearSelection_System.h"
#include "Modules/Editor/Systems/Outline_System.h"
#include "Modules/ECS/component_types.h"
#include "Utilities/IO/Level_IO.h"
#include "Engine.h"
#include <algorithm>
#include <filesystem>
#include <fstream>



/** Construct a game module.
@param	engine		reference to the engine to use. */

LevelEditor_Module::LevelEditor_Module(Engine& engine) 
	: Engine_Module(engine) 
{
}

void LevelEditor_Module::initialize() noexcept
{
	Engine_Module::initialize();
	m_engine.getManager_Messages().statement("Loading Module: Level Editor...");

	// Update indicator
	*m_aliveIndicator = true;

	// UI
	m_editorInterface = std::make_shared<Editor_Interface>(m_engine, *this);

	// Gizmos
	m_mouseGizmo = std::make_shared<Mouse_Gizmo>(m_engine, *this);

	// Systems
	m_systemOutline = std::make_shared<Outline_System>(m_engine, *this);
	m_systemSelClearer = std::make_shared<ClearSelection_System>(m_engine, *this);

	// Assets
	m_shader = Shared_Shader(m_engine, "Editor\\editorCopy");
	m_shapeQuad = Shared_Auto_Model(m_engine, "quad");
	m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
		m_indirectQuad = IndirectDraw<1>((GLuint)m_shapeQuad->getSize(), 1, 0, GL_CLIENT_STORAGE_BIT);
		});

	// Preferences
	auto& preferences = m_engine.getPreferenceState();
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) {
		m_renderSize.x = (int)f;
		glTextureImage2DEXT(m_texID, GL_TEXTURE_2D, 0, GL_RGBA16F, m_renderSize.x, m_renderSize.y, 0, GL_RGBA, GL_FLOAT, 0);
		glTextureImage2DEXT(m_depthID, GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_renderSize.x, m_renderSize.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
		});
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) {
		m_renderSize.y = (int)f;
		glTextureImage2DEXT(m_texID, GL_TEXTURE_2D, 0, GL_RGBA16F, m_renderSize.x, m_renderSize.y, 0, GL_RGBA, GL_FLOAT, 0);
		glTextureImage2DEXT(m_depthID, GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_renderSize.x, m_renderSize.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
		});
	preferences.getOrSetValue(PreferenceState::Preference::E_AUTOSAVE_INTERVAL, m_autosaveInterval);
	preferences.addCallback(PreferenceState::Preference::E_AUTOSAVE_INTERVAL, m_aliveIndicator, [&](const float& f) {
		m_autosaveInterval = f;
		});
	float undoStacksize = 500.0f;
	preferences.getOrSetValue(PreferenceState::Preference::E_UNDO_STACKSIZE, undoStacksize);
	m_maxUndo = int(undoStacksize);
	preferences.addCallback(PreferenceState::Preference::E_UNDO_STACKSIZE, m_aliveIndicator, [&](const float& f) {
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
	glTextureImage2DEXT(m_depthID, GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_renderSize.x, m_renderSize.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
	glNamedFramebufferTexture(m_fboID, GL_DEPTH_STENCIL_ATTACHMENT, m_depthID, 0);
	glNamedFramebufferDrawBuffer(m_fboID, GL_COLOR_ATTACHMENT0);
}

void LevelEditor_Module::deinitialize() noexcept
{
	m_engine.getManager_Messages().statement("Unloading Module: Level Editor...");

	// Update indicator
	*m_aliveIndicator = false;
	m_editorInterface.reset();
	m_mouseGizmo.reset();
	m_systemSelClearer.reset();
	m_systemOutline.reset();

	glDeleteFramebuffers(1, &m_fboID);
	glDeleteTextures(1, &m_texID);
	glDeleteTextures(1, &m_depthID);
}

void LevelEditor_Module::frameTick(const float& deltaTime) noexcept
{
	if (m_active) {
		constexpr GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		constexpr GLfloat clearDepth = 1.0f;
		constexpr GLint clearStencil = 0;
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glDepthMask(true);
		glStencilMask(0xFF);
		glClearNamedFramebufferfv(m_fboID, GL_COLOR, 0, clearColor);
		glClearNamedFramebufferfi(m_fboID, GL_DEPTH_STENCIL, 0, clearDepth, clearStencil);
		glStencilMask(0x00);
		glDisable(GL_STENCIL_TEST);

		// Render editor interface into separate FBO
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
		m_world.updateSystem(m_systemOutline, deltaTime);
		m_mouseGizmo->frameTick(deltaTime);
		m_editorInterface->tick(deltaTime);

		// Just apply physics systems, don't update simulation
		m_engine.getModule_Physics().updateSystems(m_world, deltaTime);

		// Render world into default FBO
		m_engine.getModule_Graphics().renderWorld(m_world, deltaTime);

		// Overlay editor interface over default FBO
		if (Asset::All_Ready(m_shapeQuad, m_shader)) {
			// Set up state
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			bindTexture();
			m_shader->bind();
			m_shapeQuad->bind();
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			m_indirectQuad.drawCall();
			Shader::Release();
		}

		glDepthMask(false);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);

		// Auto-save
		if (hasUnsavedChanges()) {
			m_autoSaveCounter += deltaTime;
			if (m_autoSaveCounter > m_autosaveInterval) {
				m_autoSaveCounter -= m_autosaveInterval;
				m_engine.getManager_Messages().statement("Autosaving Map...");
				std::filesystem::path currentPath(m_currentLevelName);
				currentPath.replace_extension(".autosave");
				saveLevel_Internal(currentPath.string());
			}
		}
		else
			m_autoSaveCounter = 0.0f;
	}
}

void LevelEditor_Module::setGizmoTransform(const Transform& transform) noexcept
{
	m_mouseGizmo->setTransform(transform);
}

Transform LevelEditor_Module::getGizmoTransform() const noexcept
{
	return m_mouseGizmo->getSelectionTransform();
}

Transform LevelEditor_Module::getSpawnTransform() const noexcept
{
	return m_mouseGizmo->getSpawnTransform();
}

glm::vec3 LevelEditor_Module::getCameraPosition() const noexcept
{
	return m_engine.getModule_Graphics().getClientCamera()->EyePosition;
}

void LevelEditor_Module::toggleAddToSelection(const EntityHandle& entityHandle) noexcept
{
	auto selectionCopy = m_mouseGizmo->getSelection();

	// If the entity is already selected, de-select it
	if (std::find(selectionCopy.cbegin(), selectionCopy.cend(), entityHandle) != selectionCopy.cend())
		selectionCopy.erase(std::remove(selectionCopy.begin(), selectionCopy.end(), entityHandle));
	else
		selectionCopy.push_back(entityHandle);

	// Ensure our gizmos stay in sync
	setSelection(selectionCopy);
}

bool LevelEditor_Module::hasCopy() const noexcept
{
	return m_copiedData.size() ? true : false;
}

void LevelEditor_Module::openSceneInspector() noexcept
{
	m_editorInterface->m_uiSceneInspector->open();
}

void LevelEditor_Module::openEntityInspector() noexcept
{
	m_editorInterface->m_uiEntityInspector->open();
}

void LevelEditor_Module::openPrefabs() noexcept
{
	m_editorInterface->m_uiPrefabs->open();
}

void LevelEditor_Module::showEditor() noexcept
{
	m_active = true;
	newLevel();

	for (const auto& item : std::filesystem::recursive_directory_iterator(Engine::Get_Current_Dir() + "\\Maps\\")) {
		const auto& path = item.path();
		if (path.has_extension() && path.extension().string() == ".autosave") {
			std::dynamic_pointer_cast<RecoverDialogue>(m_editorInterface->m_uiRecoverDialogue)->setPath(path);
			m_editorInterface->m_uiRecoverDialogue->open();
			break;
		}
	}
	populateRecentList();
}

void LevelEditor_Module::exit() noexcept
{
	std::dynamic_pointer_cast<UnsavedChangesDialogue>(m_editorInterface->m_uiUnsavedDialogue)->tryPrompt([&]() {
		m_engine.goToMainMenu();
		m_currentLevelName = "My Map.bmap";
		m_unsavedChanges = false;
		m_undoStack = {};
		m_redoStack = {};
		m_active = false;
		});
}

bool LevelEditor_Module::hasUnsavedChanges() const noexcept
{
	return m_unsavedChanges;
}

ecsWorld& LevelEditor_Module::getWorld() noexcept
{
	return m_world;
}

std::string LevelEditor_Module::getMapName() const noexcept
{
	return m_currentLevelName;
}

std::deque<std::string> LevelEditor_Module::getRecentLevels() const noexcept
{
	return m_recentLevels;
}

void LevelEditor_Module::newLevel() noexcept
{
	std::dynamic_pointer_cast<UnsavedChangesDialogue>(m_editorInterface->m_uiUnsavedDialogue)->tryPrompt([&]() {
		m_world.clear();
		m_currentLevelName = "My Map.bmap";

		// Starting new level, changes will be discarded
		m_unsavedChanges = false;
		m_undoStack = {};
		m_redoStack = {};
		});
}

void LevelEditor_Module::openLevel(const std::string& name) noexcept
{
	// If the level doesn't exist, remove it from the 'recent levels' list
	if (!Level_IO::Level_Exists(name)) {
		std::dynamic_pointer_cast<MissingFileDialogue>(m_editorInterface->m_uiMissingDialogue)->notifyMissing(name);
		if (std::find(m_recentLevels.cbegin(), m_recentLevels.cend(), name) != m_recentLevels.cend())
			m_recentLevels.erase(std::remove(m_recentLevels.begin(), m_recentLevels.end(), name));
	}
	// Otherwise, try opening the level
	else {
		if (Level_IO::Import_BMap(name, m_world)) {
			m_undoStack = {};
			m_redoStack = {};
			m_unsavedChanges = false;
			m_currentLevelName = name;
			addToRecentList(name);
		}
		else
			m_engine.getManager_Messages().error("Cannot open the level: " + name);
	}
}

void LevelEditor_Module::openLevelDialogue() noexcept
{
	std::dynamic_pointer_cast<UnsavedChangesDialogue>(m_editorInterface->m_uiUnsavedDialogue)->tryPrompt([&]() {
		m_editorInterface->m_uiOpenDialogue->open();
		});
}

void LevelEditor_Module::saveLevel(const std::string& name) noexcept
{
	// Make sure the level has a valid name, otherwise open the naming dialogue
	m_currentLevelName = name;
	if (name == "")
		saveLevelDialogue();
	else {
		saveLevel_Internal(m_currentLevelName);
		addToRecentList(m_currentLevelName);

		// Delete Autosaves
		std::filesystem::path currentPath(m_currentLevelName);
		if (currentPath.has_extension() && currentPath.extension() != ".autosave") {
			currentPath.replace_extension(".autosave");
			currentPath = std::filesystem::path(Engine::Get_Current_Dir() + "\\Maps\\" + currentPath.string());
			if (std::filesystem::exists(currentPath) && !std::filesystem::is_directory(currentPath))
				std::filesystem::remove(currentPath);
		}

		m_unsavedChanges = false;
	}
}

void LevelEditor_Module::saveLevel_Internal(const std::string& name) noexcept
{
	if (Level_IO::Export_BMap(name, m_world))
		m_engine.getManager_Messages().statement("Level saved successfully.");
	else
		m_engine.getManager_Messages().error("Cannot save the level: " + name);
}

void LevelEditor_Module::saveLevel() noexcept
{
	saveLevel(m_currentLevelName);
}

void LevelEditor_Module::saveLevelDialogue() noexcept
{
	m_editorInterface->m_uiSaveDialogue->open();
}

void LevelEditor_Module::openSettingsDialogue() noexcept
{
	m_editorInterface->m_uiSettings->open();
}

bool LevelEditor_Module::canUndo() const noexcept
{
	return m_undoStack.size();
}

bool LevelEditor_Module::canRedo() const noexcept
{
	return m_redoStack.size();
}

void LevelEditor_Module::undo() noexcept
{
	if (m_undoStack.size()) {
		// Undo the last action
		m_undoStack.front()->undo();

		// Move the action onto the redo stack
		m_redoStack.push_front(m_undoStack.front());
		m_undoStack.pop_front();

		// Set unsaved changes all the time
		m_unsavedChanges = true;
	}
}

void LevelEditor_Module::redo() noexcept
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

void LevelEditor_Module::doReversableAction(const std::shared_ptr<Editor_Command>& command) noexcept
{
	// Clear the redo stack
	m_redoStack = {};

	// Perform the desired action
	command->execute();

	// Try to join the new command into the previous one if the types match
	if (m_undoStack.size() && typeid(m_undoStack.front()) == typeid(command) && m_undoStack.front()->join(command.get())) {}
	// Otherwise add action to the undo stack
	else
		m_undoStack.push_front(command);

	while (m_undoStack.size() > m_maxUndo)
		m_undoStack.pop_back();
	m_unsavedChanges = true;
}

void LevelEditor_Module::addToRecentList(const std::string& name) noexcept
{
	if (std::find(m_recentLevels.cbegin(), m_recentLevels.cend(), name) != m_recentLevels.cend())
		m_recentLevels.erase(std::remove(m_recentLevels.begin(), m_recentLevels.end(), name));
	m_recentLevels.push_front(name);
	if (m_recentLevels.size() > 15)
		m_recentLevels.resize(15);

	// Dump recent-list data to disk
	std::ofstream file(Engine::Get_Current_Dir() + "\\Maps\\recent.editor", std::ios::beg);
	if (!file.is_open())
		m_engine.getManager_Messages().error("Cannot write the recent level list to disk!");
	else
		for (const auto& level : m_recentLevels)
			file << level << "\n";
	file.close();
}

void LevelEditor_Module::populateRecentList() noexcept
{
	// Fetch recent-list data from disk
	m_recentLevels.clear();
	std::ifstream file(Engine::Get_Current_Dir() + "\\Maps\\recent.editor", std::ios::beg);
	if (!file.is_open())
		m_engine.getManager_Messages().error("Cannot read the recent level list from disk!");
	else {
		std::string level;
		while (std::getline(file, level))
			m_recentLevels.push_back(level);
	}
	file.close();
}

void LevelEditor_Module::clearSelection() noexcept
{
	struct Clear_Selection_Command final : Editor_Command {
		Engine& m_engine;
		LevelEditor_Module& m_editor;
		const std::vector<EntityHandle> m_uuids_old;
		Clear_Selection_Command(Engine& engine, LevelEditor_Module& editor) noexcept
			: m_engine(engine), m_editor(editor), m_uuids_old(m_editor.getSelection()) {}
		void switchSelection(const std::vector<EntityHandle>& uuids) noexcept {
			// Remove all selection components from world
			auto& ecsWorld = m_editor.getWorld();
			ecsWorld.updateSystem(m_editor.m_systemSelClearer.get(), 0.0f);
			m_editor.m_mouseGizmo->getSelection().clear();

			// Add selection component to new selection
			m_editor.m_mouseGizmo->setSelection(uuids);
			for (const auto& entityHandle : uuids)
				ecsWorld.makeComponent(entityHandle, Selected_Component::Runtime_ID);

			// Transform gizmo to center of group
			Transform newTransform;
			size_t count(0ull);
			glm::vec3 center(0.0f), scale(0.0f);
			for (const auto& entityHandle : uuids)
				if (auto* transform = ecsWorld.getComponent<Transform_Component>(entityHandle)) {
					center += transform->m_localTransform.m_position;
					scale += transform->m_localTransform.m_scale;
					count++;
				}
			center /= count;
			scale /= count;
			newTransform.m_position = center;
			newTransform.m_scale = scale;
			newTransform.update();
			m_editor.m_mouseGizmo->setTransform(newTransform);
		};
		virtual void execute() noexcept override final {
			// Remove all selection components from world
			m_editor.getWorld().updateSystem(m_editor.m_systemSelClearer.get(), 0.0f);
			m_editor.m_mouseGizmo->getSelection().clear();
		}
		virtual void undo() noexcept override final {
			// Remove all selection components from world
			auto& ecsWorld = m_editor.getWorld();
			ecsWorld.updateSystem(m_editor.m_systemSelClearer.get(), 0.0f);
			m_editor.m_mouseGizmo->getSelection().clear();

			// Add selection component to new selection
			m_editor.m_mouseGizmo->setSelection(m_uuids_old);
			for (const auto& entityHandle : m_uuids_old)
				ecsWorld.makeComponent(entityHandle, Selected_Component::Runtime_ID);

			// Transform gizmo to center of group
			Transform newTransform;
			size_t count(0ull);
			glm::vec3 center(0.0f), scale(0.0f);
			for (const auto& entityHandle : m_uuids_old)
				if (auto* transform = ecsWorld.getComponent<Transform_Component>(entityHandle)) {
					center += transform->m_localTransform.m_position;
					scale += transform->m_localTransform.m_scale;
					count++;
				}
			center /= count;
			scale /= count;
			newTransform.m_position = center;
			newTransform.m_scale = scale;
			newTransform.update();
			m_editor.m_mouseGizmo->setTransform(newTransform);
		}
		virtual bool join(Editor_Command* other) noexcept override final {
			if (auto newCommand = dynamic_cast<Clear_Selection_Command*>(other))
				return true;
			return false;
		}
	};

	if (getSelection().size())
		doReversableAction(std::make_shared<Clear_Selection_Command>(m_engine, *this));
}

void LevelEditor_Module::selectAll() noexcept
{
	setSelection(getWorld().getEntityHandles());
}

void LevelEditor_Module::setSelection(const std::vector<EntityHandle>& handles) noexcept
{
	struct Set_Selection_Command final : Editor_Command {
		Engine& m_engine;
		LevelEditor_Module& m_editor;
		std::vector<EntityHandle> m_uuids_new, m_uuids_old;
		Set_Selection_Command(Engine& engine, LevelEditor_Module& editor, const std::vector<EntityHandle>& newSelection) noexcept
			: m_engine(engine), m_editor(editor), m_uuids_new(newSelection), m_uuids_old(m_editor.getSelection()) {}
		void switchSelection(const std::vector<EntityHandle>& uuids) noexcept {
			// Remove all selection components from world
			auto& ecsWorld = m_editor.getWorld();
			ecsWorld.updateSystem(m_editor.m_systemSelClearer.get(), 0.0f);
			m_editor.m_mouseGizmo->getSelection().clear();

			// Add selection component to new selection
			m_editor.m_mouseGizmo->setSelection(uuids);
			for (const auto& entityHandle : uuids)
				ecsWorld.makeComponent(entityHandle, Selected_Component::Runtime_ID);

			// Transform gizmo to center of group
			Transform newTransform;
			size_t count(0ull);
			glm::vec3 center(0.0f), scale(0.0f);
			for (const auto& entityHandle : uuids)
				if (auto* transform = ecsWorld.getComponent<Transform_Component>(entityHandle)) {
					center += transform->m_localTransform.m_position;
					scale += transform->m_localTransform.m_scale;
					count++;
				}
			center /= count;
			scale /= count;
			newTransform.m_position = center;
			newTransform.m_scale = scale;
			newTransform.update();
			m_editor.m_mouseGizmo->setTransform(newTransform);
		};
		virtual void execute() noexcept override final {
			switchSelection(m_uuids_new);
		}
		virtual void undo() noexcept override final {
			switchSelection(m_uuids_old);
		}
		virtual bool join(Editor_Command* other) noexcept override final {
			if (auto newCommand = dynamic_cast<Set_Selection_Command*>(other)) {
				// Join the 2 'new' sets together, make sure it's unique
				m_uuids_new.insert(m_uuids_new.begin(), newCommand->m_uuids_new.cbegin(), newCommand->m_uuids_new.cend());
				m_uuids_new.erase(std::unique(m_uuids_new.begin(), m_uuids_new.end()), m_uuids_new.end());
				return true;
			}
			return false;
		}
	};

	if (handles.size())
		doReversableAction(std::make_shared<Set_Selection_Command>(m_engine, *this, handles));
}

const std::vector<EntityHandle>& LevelEditor_Module::getSelection() const noexcept
{
	return m_mouseGizmo->getSelection();
}

void LevelEditor_Module::mergeSelection() noexcept
{
	struct Merge_Selection_Command final : Editor_Command {
		Engine& m_engine;
		LevelEditor_Module& m_editor;
		std::vector<EntityHandle> m_uuids;
		Merge_Selection_Command(Engine& engine, LevelEditor_Module& editor) noexcept
			: m_engine(engine), m_editor(editor), m_uuids(m_editor.getSelection()) {}
		virtual void execute() noexcept override final {
			auto& ecsWorld = m_editor.getWorld();
			// Find the root element
			const auto& root = m_uuids[0];
			if (root.isValid()) {
				// Parent remaining entities in the selection to the root
				for (size_t x = 1ull, selSize = m_uuids.size(); x < selSize; ++x)
					if (const auto& entityHandle = m_uuids[x])
						ecsWorld.parentEntity(root, entityHandle);
				m_editor.m_mouseGizmo->getSelection() = { root };
			}
		}
		virtual void undo() noexcept override final {
			auto& ecsWorld = m_editor.getWorld();
			// Find the root element
			if (auto* root = ecsWorld.getEntity(m_uuids[0])) {
				// Un-parent remaining entities from the root
				for (size_t x = 1ull, selSize = m_uuids.size(); x < selSize; ++x)
					if (const auto& entityHandle = m_uuids[x])
						ecsWorld.unparentEntity(entityHandle);
			}
		}
		virtual bool join(Editor_Command* other) noexcept override final {
			if (auto newCommand = dynamic_cast<Merge_Selection_Command*>(other)) {
				// If root is the same, continue
				if (m_uuids[0] == newCommand->m_uuids[0]) {
					// Join the 2 'new' sets together, make sure it's unique
					m_uuids.insert(m_uuids.begin(), newCommand->m_uuids.cbegin(), newCommand->m_uuids.cend());
					m_uuids.erase(std::unique(m_uuids.begin(), m_uuids.end()), m_uuids.end());
				}
				return true;
			}
			return false;
		}
	};

	if (m_mouseGizmo->getSelection().size())
		doReversableAction(std::make_shared<Merge_Selection_Command>(m_engine, *this));
}

void LevelEditor_Module::groupSelection() noexcept
{
	struct Group_Selection_Command final : Editor_Command {
		Engine& m_engine;
		LevelEditor_Module& m_editor;
		const std::vector<EntityHandle> m_uuids;
		EntityHandle m_rootUUID;
		Group_Selection_Command(Engine& engine, LevelEditor_Module& editor) noexcept
			: m_engine(engine), m_editor(editor), m_uuids(m_editor.getSelection()) {}
		virtual void execute() noexcept override final {
			// Determine a new central transform for the whole group
			auto& ecsWorld = m_editor.getWorld();
			Transform_Component rootTransform;
			size_t posCount(0ull);
			for (const auto& entityHandle : m_uuids)
				if (const auto& transform = ecsWorld.getComponent<Transform_Component>(entityHandle)) {
					rootTransform.m_localTransform.m_position += transform->m_localTransform.m_position;
					posCount++;
				}
			rootTransform.m_localTransform.m_position /= float(posCount);
			rootTransform.m_localTransform.update();
			rootTransform.m_worldTransform = rootTransform.m_localTransform;

			// Make a new root entity for the selection
			ecsBaseComponent* entityComponents[] = { &rootTransform };
			m_rootUUID = ecsWorld.makeEntity(entityComponents, 1ull, "Group", m_rootUUID);

			// Offset children by new center position
			for (auto& uuid : m_uuids)
				ecsWorld.parentEntity(m_rootUUID, uuid);
		}
		virtual void undo() noexcept override final {
			auto& ecsWorld = m_editor.getWorld();
			auto& selection = m_editor.m_mouseGizmo->getSelection();
			selection.clear();
			if (m_rootUUID != EntityHandle()) {
				for (const auto& child : ecsWorld.getEntityHandles(m_rootUUID)) {
					ecsWorld.unparentEntity(child);
					selection.push_back(child);
				}
				ecsWorld.removeEntity(m_rootUUID);
			}
		}
	};

	if (m_mouseGizmo->getSelection().size())
		doReversableAction(std::make_shared<Group_Selection_Command>(m_engine, *this));
}

void LevelEditor_Module::ungroupSelection() noexcept
{
	struct Ungroup_Selection_Command final : Editor_Command {
		Engine& m_engine;
		LevelEditor_Module& m_editor;
		const std::vector<EntityHandle> m_uuids;
		std::vector<std::vector<EntityHandle>> m_children;
		Ungroup_Selection_Command(Engine& engine, LevelEditor_Module& editor) noexcept
			: m_engine(engine), m_editor(editor), m_uuids(m_editor.getSelection()) {
			const auto& ecsWorld = m_editor.getWorld();
			for (const auto& entityHandle : m_uuids)
				m_children.push_back(ecsWorld.getEntityHandles(entityHandle));			
		}
		virtual void execute() noexcept override final {
			auto& ecsWorld = m_editor.getWorld();
			for (const auto& entityHandle : m_uuids)
				for (const auto& childHandle : ecsWorld.getEntityHandles(entityHandle))
					ecsWorld.unparentEntity(childHandle);
		}
		virtual void undo() noexcept override final {
			auto& ecsWorld = m_editor.getWorld();
			size_t childIndex(0ull);
			for (const auto& enityUUID : m_uuids)
				for (const auto& childUUID : m_children[childIndex++])
					ecsWorld.parentEntity(enityUUID, childUUID);
		}
	};

	if (m_mouseGizmo->getSelection().size())
		doReversableAction(std::make_shared<Ungroup_Selection_Command>(m_engine, *this));
}

void LevelEditor_Module::makePrefab() noexcept
{
	std::dynamic_pointer_cast<Prefabs>(m_editorInterface->m_uiPrefabs)->addPrefab(getWorld().serializeEntities(getSelection()));
}

void LevelEditor_Module::cutSelection() noexcept
{
	copySelection();
	deleteSelection();
}

void LevelEditor_Module::copySelection() noexcept
{
	m_copiedData.clear();
	auto& ecsWorld = getWorld();
	for (const auto& entityHandle : getSelection()) {
		const auto entData = ecsWorld.serializeEntity(entityHandle);
		m_copiedData.insert(m_copiedData.end(), entData.begin(), entData.end());
	}
}

void LevelEditor_Module::paste() noexcept
{
	if (m_copiedData.size())
		addEntity(m_copiedData);
}

void LevelEditor_Module::deleteSelection() noexcept
{
	struct Delete_Selection_Command final : Editor_Command {
		Engine& m_engine;
		LevelEditor_Module& m_editor;
		const std::vector<char> m_data;
		const std::vector<EntityHandle> m_uuids;
		Delete_Selection_Command(Engine& engine, LevelEditor_Module& editor, const std::vector<EntityHandle>& selection) noexcept
			: m_engine(engine), m_editor(editor), m_data(editor.getWorld().serializeEntities(selection)), m_uuids(selection) {}
		virtual void execute() noexcept override final {
			auto& ecsWorld = m_editor.getWorld();
			for (const auto& entityHandle : m_uuids)
				ecsWorld.removeEntity(entityHandle);
		}
		virtual void undo() noexcept override final {
			auto& ecsWorld = m_editor.getWorld();
			size_t dataRead(0ull), uuidIndex(0ull);
			while (dataRead < m_data.size() && uuidIndex < m_uuids.size())
				ecsWorld.deserializeEntity(m_data, m_data.size(), dataRead, EntityHandle(), m_uuids[uuidIndex]);
		}
	};

	auto& selection = m_mouseGizmo->getSelection();
	if (selection.size())
		doReversableAction(std::make_shared<Delete_Selection_Command>(m_engine, *this, selection));
}

void LevelEditor_Module::makeComponent(const EntityHandle& entityHandle, const char* name) noexcept
{
	struct Spawn_Component_Command final : Editor_Command {
		Engine& m_engine;
		LevelEditor_Module& m_editor;
		const EntityHandle m_entityHandle;
		const char* m_componentName;
		ComponentHandle m_componentHandle;
		Spawn_Component_Command(Engine& engine, LevelEditor_Module& editor, const EntityHandle& entityHandle, const char* name) noexcept
			: m_engine(engine), m_editor(editor), m_entityHandle(entityHandle), m_componentName(name) {}
		virtual void execute() noexcept override final {
			auto& ecsWorld = m_editor.getWorld();
			if (const auto& componentID = ecsWorld.nameToComponentID(m_componentName))
				m_componentHandle = ecsWorld.makeComponent(m_entityHandle, *componentID, nullptr, m_componentHandle);
		}
		virtual void undo() noexcept override final {
			auto& ecsWorld = m_editor.getWorld();
			if (const auto& componentID = ecsWorld.nameToComponentID(m_componentName)) {
				for (auto& component : ecsWorld.getEntity(m_entityHandle)->m_components) {
					const auto& [compID, fn, compHandle] = component;
					if (compID == componentID) {
						ecsWorld.removeEntityComponent(m_entityHandle, compID);
						break;
					}
				}
			}
		}
	};

	doReversableAction(std::make_shared<Spawn_Component_Command>(m_engine, *this, entityHandle, name));
}

void LevelEditor_Module::deleteComponent(const EntityHandle& entityHandle, const int& componentID) noexcept
{
	struct Delete_Component_Command final : Editor_Command {
		Engine& m_engine;
		LevelEditor_Module& m_editor;
		const EntityHandle m_entityHandle;
		const ComponentHandle m_componentHandle;
		const int m_componentID;
		std::vector<char> m_componentData;
		Delete_Component_Command(Engine& engine, LevelEditor_Module& editor, const EntityHandle& entityHandle, const ComponentHandle& componentHandle, const int& componentID) noexcept
			: m_engine(engine), m_editor(editor), m_entityHandle(entityHandle), m_componentHandle(componentHandle), m_componentID(componentID) {
			auto& ecsWorld = m_editor.getWorld();
			if (const auto& component = ecsWorld.getComponent(m_entityHandle, m_componentID))
				m_componentData = component->to_buffer();
		}
		virtual void execute() noexcept override final {
			m_editor.getWorld().removeEntityComponent(m_entityHandle, m_componentID);
		}
		virtual void undo() noexcept override final {
			if (m_componentData.size()) {
				size_t dataRead(0ull);
				const auto& copy = ecsBaseComponent::from_buffer(m_componentData, dataRead);
				m_editor.getWorld().makeComponent(m_entityHandle, copy.get(), m_componentHandle);
			}
		}
	};

	if (const auto* component = getWorld().getComponent(entityHandle, componentID))
		doReversableAction(std::make_shared<Delete_Component_Command>(m_engine, *this, entityHandle, component->m_handle, componentID));
}

void LevelEditor_Module::addEntity(const std::vector<char>& entityData, const EntityHandle& parentUUID) noexcept
{
	struct Spawn_Command final : Editor_Command {
		Engine& m_engine;
		LevelEditor_Module& m_editor;
		const std::vector<char> m_data;
		const EntityHandle m_parentUUID;
		const Transform m_cursor;
		std::vector<EntityHandle> m_uuids;
		Spawn_Command(Engine& engine, LevelEditor_Module& editor, const std::vector<char>& data, const EntityHandle& pUUID) noexcept
			: m_engine(engine), m_editor(editor), m_data(data), m_parentUUID(pUUID), m_cursor(m_editor.getSpawnTransform()) {}
		virtual void execute() noexcept override final {
			auto& ecsWorld = m_editor.getWorld();
			size_t dataRead(0ull), handleCount(0ull);
			glm::vec3 center(0.0f);
			std::vector<Transform_Component*> transformComponents;
			while (dataRead < m_data.size()) {
				// Ensure we have a vector large enough to hold all UUIDs, but maintain previous data
				m_uuids.resize(std::max<size_t>(m_uuids.size(), handleCount + 1ull));
				const auto desiredHandle = m_uuids[handleCount].isValid() ? m_uuids[handleCount] : (EntityHandle)(ecsWorld.generateUUID());
				const auto& [entityHandle, entity] = ecsWorld.deserializeEntity(m_data, m_data.size(), dataRead, m_parentUUID, desiredHandle);
				if (entityHandle.isValid() && entity) {
					if (auto* transform = ecsWorld.getComponent<Transform_Component>(entityHandle)) {
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
			for (auto* transform : transformComponents) {
				transform->m_localTransform.m_position = (transform->m_localTransform.m_position - center) + cursorPos;
				transform->m_localTransform.update();
			}
		}
		virtual void undo() noexcept override final {
			auto& ecsWorld = m_editor.getWorld();
			for (const auto& entityHandle : m_uuids)
				ecsWorld.removeEntity(entityHandle);
		}
	};

	if (entityData.size())
		doReversableAction(std::make_shared<Spawn_Command>(m_engine, *this, entityData, parentUUID));
}

void LevelEditor_Module::bindFBO() noexcept
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
}

void LevelEditor_Module::bindTexture(const GLuint& offset) noexcept
{
	glBindTextureUnit(offset, m_texID);
}

bool Editor_Command::join(Editor_Command* newerCommand) noexcept 
{
	return false;
}