#include "Modules/Editor/Editor_M.h"
#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/Editor/UI/CameraController.h"
#include "Modules/Editor/UI/RotationIndicator.h"
#include "Modules/Editor/UI/TitleBar.h"
#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/UI/Inspector.h"
#include "Modules/Editor/UI/LevelDialogue.h"
#include "Modules/Editor/Gizmos/Selection.h"
#include "Modules/Editor/Systems/ClearSelection_System.h"
#include "Modules/Editor/Systems/Wireframe_System.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Modules/World/ECS/components.h"
#include "Engine.h"


void LevelEditor_Module::initialize(Engine* engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: Level Edtior...");

	// Update indicator
	*m_aliveIndicator = true;

	// UI
	m_editorInterface = std::make_shared<Editor_Interface>(engine, this);

	// Gizmos
	m_selectionGizmo = std::make_shared<Selection_Gizmo>(engine, this);

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
	constexpr GLfloat clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	constexpr GLfloat clearDepth = 1.0f;
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glClearNamedFramebufferfv(m_fboID, GL_COLOR, 0, clearColor);
	glClearNamedFramebufferfv(m_fboID, GL_DEPTH, 0, &clearDepth);

	// Tick all tools this frame
	m_selectionGizmo->frameTick(deltaTime);
	m_engine->getModule_World().updateSystems(m_systems, deltaTime);

	glDepthMask(false);
	glDisable(GL_DEPTH_TEST);
}

void LevelEditor_Module::setGizmoTransform(const Transform& transform)
{
	m_selectionGizmo->setTransform(transform);
}

Transform LevelEditor_Module::getGizmoTransform() const
{
	return m_selectionGizmo->getTransform();
}

const glm::vec3& LevelEditor_Module::getCameraPosition() const
{
	return m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition;
}

void LevelEditor_Module::toggleAddToSelection(ecsEntity* entity)
{
	auto selectionCopy = m_selectionGizmo->getSelection();

	// If the entity is already selected, deselect it
	if (std::find(selectionCopy.cbegin(), selectionCopy.cend(), entity) != selectionCopy.cend())
		std::remove(selectionCopy.begin(), selectionCopy.end(), entity);
	else
		selectionCopy.push_back(entity);

	// Ensure our gizmos stay in sync
	setSelection(selectionCopy);
}

bool LevelEditor_Module::hasCopy() const
{
	return m_copiedData.size() ? true : false;
}

void LevelEditor_Module::showEditor()
{
	m_engine->getModule_UI().clear();
	m_engine->getModule_UI().setRootElement(m_editorInterface);
}

void LevelEditor_Module::exit()
{
	m_engine->goToMainMenu();
	m_currentLevelName = "";
}

void LevelEditor_Module::newLevel()
{
	/**@todo	check against dirty bit for 'level has unsaved changes' */
	m_engine->getModule_World().unloadWorld();
	m_currentLevelName = "";
}

void LevelEditor_Module::openLevel(const std::string& name)
{
	/**@todo	check against dirty bit for 'level has unsaved changes' */
	m_engine->getModule_World().loadWorld(name);
	m_currentLevelName = name;
}

void LevelEditor_Module::openLevelDialog()
{
	/**@todo	check against dirty bit for 'level has unsaved changes' */
	m_editorInterface->m_uiLevelDialogue->startOpenDialogue();
}

void LevelEditor_Module::saveLevel(const std::string& name)
{
	m_engine->getModule_World().saveWorld(name);
	m_currentLevelName = name;
}

void LevelEditor_Module::saveLevel()
{
	saveLevel(m_currentLevelName);
}

void LevelEditor_Module::saveLevelDialog()
{
	m_editorInterface->m_uiLevelDialogue->startSaveDialogue();
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
		m_undoStack.top()->undo();

		// Move the action onto the redo stack
		m_redoStack.push(m_undoStack.top());
		m_undoStack.pop();
	}
}

void LevelEditor_Module::redo()
{
	if (m_redoStack.size()) {
		// Redo the last action
		m_redoStack.top()->execute();

		// Push the action onto the undo stack
		m_undoStack.push(m_redoStack.top());
		m_redoStack.pop();
	}
}

void LevelEditor_Module::doReversableAction(const std::shared_ptr<Editor_Command>& command)
{
	// Clear the redo stack
	m_redoStack = {};

	// Perform the desired action
	command->execute();

	// Add action to the undo stack
	m_undoStack.push(command);
}

void LevelEditor_Module::clearSelection()
{
	struct Clear_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::vector<std::string> m_uuids_old;
		Clear_Selection_Command(Engine* engine, LevelEditor_Module* editor)
			: m_engine(engine), m_editor(editor), m_uuids_old(m_engine->getModule_World().getUUIDs(m_editor->getSelection())) {}
		void switchSelection(const std::vector<std::string>& uuids) {
			// Remove all selection components from world
			auto& world = m_engine->getModule_World();
			world.updateSystem(m_editor->m_selectionClearer.get(), 0.0f);
			m_editor->m_selectionGizmo->getSelection().clear();

			// Add selection component to new selection
			std::vector<ecsEntity*> entities = world.findEntities(uuids);
			m_editor->m_selectionGizmo->setSelection(entities);
			for each (auto * entity in entities) {
				const Selected_Component component;
				world.addComponent(entity, &component);
			}

			// Transform gizmo to center of group
			Transform newTransform;
			size_t count(0ull);
			glm::vec3 center(0.0f), scale(0.0f);
			for each (const auto & entity in entities)
				if (auto * transform = world.getComponent<Transform_Component>(entity)) {
					center += transform->m_localTransform.m_position;
					scale += transform->m_localTransform.m_scale;
					count++;
				}
			center /= count;
			scale /= count;
			newTransform.m_position = center;
			newTransform.m_scale = scale;
			newTransform.update();
			m_editor->m_selectionGizmo->setTransform(newTransform);
		};
		virtual void execute() {
			// Remove all selection components from world
			auto& world = m_engine->getModule_World();
			world.updateSystem(m_editor->m_selectionClearer.get(), 0.0f);
			m_editor->m_selectionGizmo->getSelection().clear();
		}
		virtual void undo() {
			// Remove all selection components from world
			auto& world = m_engine->getModule_World();
			world.updateSystem(m_editor->m_selectionClearer.get(), 0.0f);
			m_editor->m_selectionGizmo->getSelection().clear();

			// Add selection component to new selection
			std::vector<ecsEntity*> entities = world.findEntities(m_uuids_old);
			m_editor->m_selectionGizmo->setSelection(entities);
			for each (auto * entity in entities) {
				const Selected_Component component;
				world.addComponent(entity, &component);
			}

			// Transform gizmo to center of group
			Transform newTransform;
			size_t count(0ull);
			glm::vec3 center(0.0f), scale(0.0f);
			for each (const auto & entity in entities)
				if (auto * transform = world.getComponent<Transform_Component>(entity)) {
					center += transform->m_localTransform.m_position;
					scale += transform->m_localTransform.m_scale;
					count++;
				}
			center /= count;
			scale /= count;
			newTransform.m_position = center;
			newTransform.m_scale = scale;
			newTransform.update();
			m_editor->m_selectionGizmo->setTransform(newTransform);
		}
	};

	if (getSelection().size())
		doReversableAction(std::make_shared<Clear_Selection_Command>(m_engine, this));
}

void LevelEditor_Module::selectAll()
{
	setSelection(m_engine->getModule_World().getEntities());
}

void LevelEditor_Module::setSelection(const std::vector<ecsEntity*>& entities)
{
	struct Set_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::vector<std::string> m_uuids_new, m_uuids_old;
		Set_Selection_Command(Engine* engine, LevelEditor_Module* editor, const std::vector<ecsEntity*>& newSelection)
			: m_engine(engine), m_editor(editor), m_uuids_new(m_engine->getModule_World().getUUIDs(newSelection)), m_uuids_old(m_engine->getModule_World().getUUIDs(m_editor->getSelection())) {}
		void switchSelection(const std::vector<std::string>& uuids) {
			// Remove all selection components from world
			auto& world = m_engine->getModule_World();
			world.updateSystem(m_editor->m_selectionClearer.get(), 0.0f);
			m_editor->m_selectionGizmo->getSelection().clear();

			// Add selection component to new selection
			std::vector<ecsEntity*> entities = world.findEntities(uuids);
			m_editor->m_selectionGizmo->setSelection(entities);
			for each (auto * entity in entities) {
				const Selected_Component component;
				world.addComponent(entity, &component);
			}

			// Transform gizmo to center of group
			Transform newTransform;
			size_t count(0ull);
			glm::vec3 center(0.0f), scale(0.0f);
			for each (const auto & entity in entities)
				if (auto * transform = world.getComponent<Transform_Component>(entity)) {
					center += transform->m_localTransform.m_position;
					scale += transform->m_localTransform.m_scale;
					count++;
				}
			center /= count;
			scale /= count;
			newTransform.m_position = center;
			newTransform.m_scale = scale;
			newTransform.update();
			m_editor->m_selectionGizmo->setTransform(newTransform);
		};
		virtual void execute() {
			switchSelection(m_uuids_new);
		}
		virtual void undo() {
			switchSelection(m_uuids_old);
		}
	};

	if (entities.size())
		doReversableAction(std::make_shared<Set_Selection_Command>(m_engine, this, entities));
}

const std::vector<ecsEntity*>& LevelEditor_Module::getSelection() const
{
	return m_selectionGizmo->getSelection();
}

void LevelEditor_Module::mergeSelection()
{
	struct Merge_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::vector<std::string> m_uuids;
		Merge_Selection_Command(Engine* engine, LevelEditor_Module* editor)
			: m_engine(engine), m_editor(editor), m_uuids(m_engine->getModule_World().getUUIDs(m_editor->getSelection())) {}
		virtual void execute() {
			auto& world = m_engine->getModule_World();
			// Find the root element
			if (auto * root = world.findEntity(m_uuids[0])) {
				// Parent remaining entities in the selection to the root
				for (size_t x = 1ull, selSize = m_uuids.size(); x < selSize; ++x)
					if (auto * entity = world.findEntity(m_uuids[x]))
						world.parentEntity(root, entity);
				m_editor->m_selectionGizmo->getSelection() = { root };
			}
		}
		virtual void undo() {
			auto& world = m_engine->getModule_World();
			// Find the root element
			if (auto * root = world.findEntity(m_uuids[0])) {
				// Unparent remaining entities from the root
				for (size_t x = 1ull, selSize = m_uuids.size(); x < selSize; ++x)
					if (auto * entity = world.findEntity(m_uuids[x])) 
						world.unparentEntity(entity);				
			}
		}
	};

	auto& selection = m_selectionGizmo->getSelection();
	if (m_selectionGizmo->getSelection().size())
		doReversableAction(std::make_shared<Merge_Selection_Command>(m_engine, this));
}

void LevelEditor_Module::groupSelection()
{
	struct Group_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::vector<std::string> m_uuids;
		std::string m_rootUUID = "";
		Group_Selection_Command(Engine* engine, LevelEditor_Module* editor)
			: m_engine(engine), m_editor(editor), m_uuids(m_engine->getModule_World().getUUIDs(m_editor->getSelection())) {}
		virtual void execute() {
			// Determine a new central transform for the whole group
			auto& world = m_engine->getModule_World();
			const auto& entities = world.findEntities(m_uuids);
			Transform_Component rootTransform;
			size_t posCount(0ull);
			for each (auto & entity in entities)
				if (const auto & transform = world.getComponent<Transform_Component>(entity)) {
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
			if (m_rootUUID == "")
				m_rootUUID = root->m_uuid;

			// Offset children by new center position
			for each (auto & entity in entities)
				world.parentEntity(root, entity);
		}
		virtual void undo() {
			auto& world = m_engine->getModule_World();
			auto& selection = m_editor->m_selectionGizmo->getSelection();
			selection.clear();
			if (auto * root = world.findEntity(m_rootUUID)) {
				for each (const auto & child in root->m_children) {
					world.unparentEntity(child);
					selection.push_back(child);
				}
				world.removeEntity(root);
			}
		}
	};

	if (m_selectionGizmo->getSelection().size())
		doReversableAction(std::make_shared<Group_Selection_Command>(m_engine, this));
}

void LevelEditor_Module::ungroupSelection()
{
	struct Ungroup_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::vector<std::string> m_uuids;
		std::vector<std::vector<std::string>> m_children;
		Ungroup_Selection_Command(Engine* engine, LevelEditor_Module* editor)
			: m_engine(engine), m_editor(editor), m_uuids(m_engine->getModule_World().getUUIDs(m_editor->getSelection())) {
			auto& world = m_engine->getModule_World();
			for each (const auto & entity in world.findEntities(m_uuids)) {
				std::vector<std::string> childrenUUIDS;
				for each (const auto & child in entity->m_children)
					childrenUUIDS.push_back(child->m_uuid);			
				m_children.push_back(childrenUUIDS);
			}
		}
		virtual void execute() {
			auto& world = m_engine->getModule_World();
			for each (const auto & entity in  world.findEntities(m_uuids))
				for each (const auto & child in entity->m_children) 
					world.unparentEntity(child);			
		}
		virtual void undo() {
			auto& world = m_engine->getModule_World();
			const auto& entities = world.findEntities(m_uuids);
			size_t childIndex(0ull);
			for each (const auto & entity in entities) 
				for each (const auto & child in world.findEntities(m_children[childIndex++])) 
					world.parentEntity(entity, child);
		}
	};

	if (m_selectionGizmo->getSelection().size())
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
	for each (const auto & entity in getSelection()) {
		const auto entData = world.serializeEntity(entity);
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
		const std::vector<std::string> m_uuids;
		Delete_Selection_Command(Engine* engine, const std::vector<ecsEntity*>& selection)
			: m_engine(engine), m_data(m_engine->getModule_World().serializeEntities(selection)), m_uuids(m_engine->getModule_World().getUUIDs(selection)) {}
		virtual void execute() {
			auto& world = m_engine->getModule_World();
			for each (auto * entity in world.findEntities(m_uuids))
				world.removeEntity(entity);
		}
		virtual void undo() {
			auto& world = m_engine->getModule_World();
			size_t dataRead(0ull), uuidIndex(0ull);
			while (dataRead < m_data.size())
				world.deserializeEntity(m_data.data(), m_data.size(), dataRead);			
		}
	};

	auto& selection = m_selectionGizmo->getSelection();
	if (selection.size())
		doReversableAction(std::make_shared<Delete_Selection_Command>(m_engine, selection));
}

void LevelEditor_Module::moveSelection(const glm::vec3& newPosition)
{
	struct Move_Selection_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const glm::vec3 m_oldPosition, m_newPosition;
		const std::vector<std::string> m_uuids;
		Move_Selection_Command(Engine* engine, LevelEditor_Module* editor, const glm::vec3& newPosition)
			: m_engine(engine), m_editor(editor), m_oldPosition(m_editor->m_selectionGizmo->getTransform().m_position), m_newPosition(newPosition), m_uuids(m_engine->getModule_World().getUUIDs(m_editor->getSelection())) {}
		void move(const glm::vec3& position) {
			auto& world = m_engine->getModule_World();
			const auto& selection = world.findEntities(m_uuids);
			std::vector<Transform_Component*> transformComponents;
			glm::vec3 center(0.0f);
			for each (const auto & entity in selection)
				if (auto * transform = world.getComponent<Transform_Component>(entity)) {
					transformComponents.push_back(transform);
					center += transform->m_localTransform.m_position;
				}
			center /= transformComponents.size();
			for each (auto * transform in transformComponents) {
				transform->m_localTransform.m_position = (transform->m_localTransform.m_position - center) + position;
				transform->m_localTransform.update();
			}

			auto gizmoTransform = m_editor->m_selectionGizmo->getTransform();
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
		const std::vector<std::string> m_uuids;
		Rotate_Selection_Command(Engine* engine, LevelEditor_Module* editor, const glm::quat& newRotation)
			: m_engine(engine), m_editor(editor), m_newRotation(newRotation), m_uuids(m_engine->getModule_World().getUUIDs(m_editor->getSelection())) {}
		void rotate(const glm::quat& rotation) {
			auto& world = m_engine->getModule_World();
			const auto& selection = world.findEntities(m_uuids);
			std::vector<Transform_Component*> transformComponents;
			glm::vec3 center(0.0f);
			for each (const auto & entity in selection)
				if (auto * transform = world.getComponent<Transform_Component>(entity)) {
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

			auto gizmoTransform = m_editor->m_selectionGizmo->getTransform();
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
		const std::vector<std::string> m_uuids;
		Scale_Selection_Command(Engine* engine, LevelEditor_Module* editor, const glm::vec3& newRotation)
			: m_engine(engine), m_editor(editor), m_oldScale(m_editor->m_selectionGizmo->getTransform().m_scale), m_newScale(newRotation), m_uuids(m_engine->getModule_World().getUUIDs(m_editor->getSelection())) {}
		void scale(const glm::vec3& scale) {
			auto& world = m_engine->getModule_World();
			const auto& selection = world.findEntities(m_uuids);
			std::vector<Transform_Component*> transformComponents;
			glm::vec3 center(0.0f);
			for each (const auto & entity in selection)
				if (auto * transform = world.getComponent<Transform_Component>(entity)) {
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
			auto gizmoTransform = m_editor->m_selectionGizmo->getTransform();
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

void LevelEditor_Module::addComponent(ecsEntity* handle, const char* name)
{
	struct Spawn_Component_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::string m_entityUUID;
		const char* m_componentName;
		Spawn_Component_Command(Engine* engine, LevelEditor_Module* editor, const ecsEntity* handle, const char* name)
			: m_engine(engine), m_editor(editor), m_entityUUID(handle->m_uuid), m_componentName(name) {}
		virtual void execute() {
			if (const auto & [templateComponent, componentID, componentSize] = BaseECSComponent::findTemplate(m_componentName); templateComponent != nullptr) {
				auto& world = m_engine->getModule_World();
				auto* clone = templateComponent->clone();
				world.addComponent(world.findEntity(m_entityUUID), clone);
				delete clone;
			}
		}
		virtual void undo() {
			auto& world = m_engine->getModule_World();
			if (const auto & [templateComponent, componentID, componentSize] = BaseECSComponent::findTemplate(m_componentName); templateComponent != nullptr)
				if (auto * entity = world.findEntity(m_entityUUID))
					for each (auto & component in entity->m_components)
						if (component.first == componentID) {
							world.removeComponent(entity, component.first);
							break;
						}
		}
	};

	doReversableAction(std::make_shared<Spawn_Component_Command>(m_engine, this, handle, name));
}

void LevelEditor_Module::deleteComponent(ecsEntity* handle, const int& componentID)
{
	struct Delete_Component_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::string m_entityUUID;
		std::vector<char> m_componentData;
		const int m_componentID;
		Delete_Component_Command(Engine* engine, LevelEditor_Module* editor, const ecsEntity* handle, const int componentID)
			: m_engine(engine), m_editor(editor), m_entityUUID(handle->m_uuid), m_componentID(componentID) {
			auto& world = m_engine->getModule_World();
			if (auto * entity = world.findEntity(m_entityUUID))
				if (const auto & component = world.getComponent(entity, m_componentID))
					m_componentData = world.serializeComponent(component);
			
		}
		virtual void execute() {
			auto& world = m_engine->getModule_World();
			if (auto * entity = world.findEntity(m_entityUUID))
				world.removeComponent(entity, m_componentID);			
		}
		virtual void undo() {
			auto& world = m_engine->getModule_World();
			if (m_componentData.size()) {
				size_t dataRead(0ull);
				auto copy = world.deserializeComponent(m_componentData.data(), m_componentData.size(), dataRead);
				world.addComponent(world.findEntity(m_entityUUID), copy.first);
				delete copy.first;
			}
		}
	};

	doReversableAction(std::make_shared<Delete_Component_Command>(m_engine, this, handle, componentID));
}

void LevelEditor_Module::addEntity(const std::vector<char>& entityData, const std::string& parentUUID)
{
	struct Spawn_Command : Editor_Command {
		Engine* m_engine;
		LevelEditor_Module* m_editor;
		const std::vector<char> m_data;
		const std::string m_parentUUID;
		const Transform m_cursor;
		std::vector<std::string> m_uuids;
		Spawn_Command(Engine* engine, LevelEditor_Module* editor, const std::vector<char>& data, const std::string& pUUID)
			: m_engine(engine), m_editor(editor), m_data(data), m_parentUUID(pUUID), m_cursor(m_editor->getGizmoTransform()) {}
		virtual void execute() {
			auto& world = m_engine->getModule_World();
			size_t dataRead(0ull), uuidIndex(0ull);
			glm::vec3 center(0.0f);
			std::vector<Transform_Component*> transformComponents;
			std::vector<ecsEntity*> entities;
			while (dataRead < m_data.size()) {
				if (auto * entity = world.deserializeEntity(m_data.data(), m_data.size(), dataRead)) {
					if (auto * transform = world.getComponent<Transform_Component>(entity)) {
						transformComponents.push_back(transform);
						center += transform->m_localTransform.m_position;
					}
					entities.push_back(entity);
				}
			}
			// First time around, save the generated uuids
			if (m_uuids.size() != entities.size()) {
				for each (const auto & entity in entities)
					m_uuids.push_back(entity->m_uuid);
			}
			// Every other time, recover the first uuids used for future undo/redo actions
			else {
				size_t uuidIndex(0ull);
				for each (auto & entity in entities)
					entity->m_uuid = m_uuids[uuidIndex++];
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
			for each (auto * entity in world.findEntities(m_uuids))
				world.removeEntity(entity);
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