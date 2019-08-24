#include "Modules/Editor/Editor_M.h"
#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/Editor/UI/CameraController.h"
#include "Modules/Editor/UI/RotationIndicator.h"
#include "Modules/Editor/UI/TitleBar.h"
#include "Modules/Editor/UI/Prefabs.h"
#include "Modules/Editor/UI/Inspector.h"
#include "Modules/Editor/UI/LevelDialogue.h"
#include "Modules/Editor/Gizmos/Selection.h"
#include "Modules/Editor/Systems/Wireframe_System.h"
#include "Modules/UI/dear imgui/imgui.h"
#include "Modules/World/ECS/components.h"
#include "Engine.h"


void LevelEditor_Module::initialize(Engine * engine)
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
	m_systems.addSystem(new Wireframe_System(engine));

	// Preferences
	auto & preferences = engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
		m_renderSize.x = (int)f;
		glTextureImage2DEXT(m_texID, GL_TEXTURE_2D, 0, GL_RGBA16F, m_renderSize.x, m_renderSize.y, 0, GL_RGBA, GL_FLOAT, 0);
		glTextureImage2DEXT(m_depthID, GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, m_renderSize.x, m_renderSize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
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

void LevelEditor_Module::frameTick(const float & deltaTime)
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

void LevelEditor_Module::setGizmoTransform(const Transform & transform)
{	
	// Update all gizmos we support
	m_selectionGizmo->setTransform(transform);
}

glm::vec3 LevelEditor_Module::getGizmoPosition() const
{
	return m_selectionGizmo->getTransform().m_position;
}

const glm::vec3 & LevelEditor_Module::getCameraPosition() const
{
	return m_engine->getModule_Graphics().getClientCamera()->get()->EyePosition;
}

void LevelEditor_Module::toggleAddToSelection(ecsEntity* entity)
{
	auto & selection = m_selectionGizmo->getSelection();
	// If the entity is already selected, deselect it
	if (std::find(selection.cbegin(), selection.cend(), entity) != selection.cend())
		std::remove(selection.begin(), selection.end(), entity);
	else
		selection.push_back(entity);

	// Ensure our gizmos stay in sync
	setSelection(selection);
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
	m_engine->getModule_World().unloadWorld();
	m_currentLevelName = "";
}

void LevelEditor_Module::openLevel(const std::string & name)
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

void LevelEditor_Module::saveLevel(const std::string & name)
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

void LevelEditor_Module::undo()
{
	/**@todo	undo/redo */
	/**@todo*/
}

void LevelEditor_Module::redo()
{
	/**@todo	undo/redo */
	/**@todo*/
}

void LevelEditor_Module::clearSelection()
{
	m_selectionGizmo->getSelection().clear();
}

void LevelEditor_Module::selectAll()
{
	setSelection(m_engine->getModule_World().getEntities());
}

void LevelEditor_Module::setSelection(const std::vector<ecsEntity*>& entities)
{
	m_selectionGizmo->setSelection(entities);
}

const std::vector<ecsEntity*>& LevelEditor_Module::getSelection() const
{
	return m_selectionGizmo->getSelection();
}

void LevelEditor_Module::mergeSelection()
{
	auto& world = m_engine->getModule_World();
	auto& selection = m_selectionGizmo->getSelection();
	if (selection.size()) {
		// Find the root element
		const auto & root = selection[0];

		// Parent remaining entities in the selection to the root
		for (size_t x = 1ull, selSize = selection.size(); x < selSize; ++x) 
			world.parentEntity(root, selection[x]);		

		// Switch the selection to the root entity
		selection = { root };
	}
}

void LevelEditor_Module::groupSelection()
{
	auto& world = m_engine->getModule_World();
	auto& selection = m_selectionGizmo->getSelection();
	if (selection.size()) {
		// Determine a new central transform for the whole group
		Transform_Component rootTransform;
		size_t posCount(0ull);
		for each (auto & entity in selection)
			if (const auto & transform = world.getComponent<Transform_Component>(entity)) {
				rootTransform.m_localTransform.m_position += transform->m_localTransform.m_position;
				posCount++;
			}		
		rootTransform.m_localTransform.m_position /= float(posCount);
		rootTransform.m_localTransform.update();
		rootTransform.m_worldTransform = rootTransform.m_localTransform;

		// Make a new -root- entity for the selection
		BaseECSComponent* entityComponents[] = { &rootTransform };
		int types[] = { Transform_Component::ID };
		auto root = world.makeEntity(entityComponents, types, 1ull, "Group", selection[0]->m_parent);

		// Offset children by new center position
		for each (auto & entity in selection)
			world.parentEntity(root, entity);

		// Switch the selection to the root entity
		selection = { root };
	}
}

void LevelEditor_Module::ungroupSelection()
{
	auto& world = m_engine->getModule_World();
	auto& selection = m_selectionGizmo->getSelection();
	std::vector<ecsEntity*> newSelection;

	for each (const auto & root in selection) 
		world.unparentEntity(root);		
}

void LevelEditor_Module::makePrefab()
{
	m_editorInterface->m_uiPrefabs->makePrefab(getSelection());
}

void LevelEditor_Module::cutSelection()
{
	/**@todo	undo/redo */
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
	/**@todo	undo/redo */
	auto& world = m_engine->getModule_World();
	if (m_copiedData.size()) {
		size_t dataRead(0ull);
		glm::vec3 center(0.0f);
		std::vector<Transform_Component*> transformComponents;
		while (dataRead < m_copiedData.size()) {
			if (auto *entity = world.deserializeEntity(m_copiedData.data(), m_copiedData.size(), dataRead, nullptr))
				if (auto * transform = world.getComponent<Transform_Component>(entity)) {
					transformComponents.push_back(transform);
					center += transform->m_localTransform.m_position;
				}
		}

		// Treat entity collection as a group
		// Move the group to world origin, then transform to 3D cursor
		center /= transformComponents.size();
		const auto cursorPos = getGizmoPosition();
		for each (auto * transform in transformComponents) {
			transform->m_localTransform.m_position = (transform->m_localTransform.m_position - center) + cursorPos;
			transform->m_localTransform.update();
		}
	}
}

void LevelEditor_Module::moveSelection(const glm::vec3& newPosition)
{
	auto& world = m_engine->getModule_World();
	std::vector<Transform_Component*> transformComponents;
	glm::vec3 center(0.0f);
	for each (const auto & entity in getSelection())
		if (auto * transform = world.getComponent<Transform_Component>(entity)) {
			transformComponents.push_back(transform);
			center += transform->m_localTransform.m_position;
		}
	center /= transformComponents.size();
	for each (auto * transform in transformComponents) {
		transform->m_localTransform.m_position = (transform->m_localTransform.m_position - center) + newPosition;
		transform->m_localTransform.update();
	}
}

void LevelEditor_Module::scaleSelection(const glm::vec3& newScale)
{
	auto& world = m_engine->getModule_World();
	std::vector<Transform_Component*> transformComponents;
	glm::vec3 center(0.0f);
	for each (const auto & entity in getSelection())
		if (auto * transform = world.getComponent<Transform_Component>(entity)) {
			transformComponents.push_back(transform);
			center += transform->m_localTransform.m_position;
		}
	center /= transformComponents.size();
	for each (auto * transform in transformComponents) {
		//transform->m_localTransform.m_position = (transform->m_localTransform.m_position - center) + newPosition;
		transform->m_localTransform.m_scale = newScale;
		transform->m_localTransform.update();
	}
}

void LevelEditor_Module::deleteSelection()
{
	/**@todo	undo/redo */
	for each (const auto & entity in getSelection())
		m_engine->getModule_World().removeEntity(entity);
}

void LevelEditor_Module::deleteComponent(ecsEntity* handle, const int& componentID)
{
	/**@todo	undo/redo */
	m_engine->getModule_World().removeComponent(handle, componentID);
}

void LevelEditor_Module::addComponent(ecsEntity* handle, const char * name)
{
	if (const auto & [templateComponent, componentID, componentSize] = BaseECSComponent::findTemplate(name); templateComponent != nullptr) {
		auto * clone = templateComponent->clone();
		m_engine->getModule_World().addComponent(handle, clone);
		delete clone;
	}
}

void LevelEditor_Module::bindFBO()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
}

void LevelEditor_Module::bindTexture(const GLuint & offset)
{
	glBindTextureUnit(offset, m_texID);
}