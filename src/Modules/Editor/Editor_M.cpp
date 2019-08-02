#include "Modules/Editor/Editor_M.h"
#include "Modules/Editor/UI/Editor_Interface.h"
#include "Modules/UI/dear imgui/imgui.h"
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
	m_selectionGizmo = std::make_unique<Selection_Gizmo>(engine, this);

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
	m_selectionGizmo->frameTick(deltaTime);

	glDepthMask(false);
	glDisable(GL_DEPTH_TEST);
}

void LevelEditor_Module::setGizmoPosition(const glm::vec3 & position)
{	
	// Update all gizmos we support
	m_selectionGizmo->setPosition(position);
	// m_translateGizmo->setPosition(position);
	// m_scaleGizmo->setPosition(position);
	// m_rotateGizmo->setPosition(position);
}

glm::vec3 LevelEditor_Module::getGizmoPosition() const
{
	return m_selectionGizmo->getPosition();
}

void LevelEditor_Module::setSelection(const std::vector<EntityHandle>& entities)
{
	m_selectionGizmo->setSelection(entities);
}

const std::vector<EntityHandle> & LevelEditor_Module::getSelection() const
{
	return m_selectionGizmo->getSelection();
}

void LevelEditor_Module::toggleAddToSelection(const EntityHandle& entity)
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

void LevelEditor_Module::showEditor()
{
	m_engine->getModule_UI().clear();
	m_engine->getModule_UI().setRootElement(m_editorInterface);
	openLevel("a.bmap");
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
	m_engine->getModule_World().loadWorld(name);
	m_currentLevelName = name;
}

void LevelEditor_Module::openLevelDialog()
{
	openLevel(m_currentLevelName);
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
	/**@todo*/
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

void LevelEditor_Module::cutSelection()
{
	/**@todo	undo/redo */
	/**@todo*/
}

void LevelEditor_Module::copySelection()
{
	/**@todo	undo/redo */
	/**@todo*/
}

void LevelEditor_Module::paste()
{
	/**@todo	undo/redo */
	/**@todo*/
}

void LevelEditor_Module::deleteSelection()
{
	/**@todo	undo/redo */
	/**@todo*/
}

void LevelEditor_Module::deleteComponent(const EntityHandle& handle, const int& componentID)
{
	/**@todo	undo/redo */
	m_engine->getModule_World().removeComponent(handle, componentID);
}

void LevelEditor_Module::addComponent(const EntityHandle& handle, const char * name)
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