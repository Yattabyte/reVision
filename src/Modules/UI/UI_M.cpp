#include "Modules/UI/UI_M.h"
#include "Modules/UI/Macro Elements/StartMenu.h"
#include "Engine.h"


void UI_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: User Interface...");

	// Preferences
	auto & preferences = m_engine->getPreferenceState();
	constexpr static auto calcOthoProj = [](const glm::ivec2 & renderSize, StaticBuffer & projectionBuffer) {
		const glm::mat4 proj = glm::ortho<float>(0.0f, (float)renderSize.x, 0.0f, (float)renderSize.y, -1.0f, 1.0f);
		projectionBuffer.write(0, sizeof(glm::mat4), &proj[0][0]);
	};
	m_projectionBuffer = StaticBuffer(sizeof(glm::mat4), 0, GL_DYNAMIC_STORAGE_BIT);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
		m_renderSize.x = (int)f;
		calcOthoProj(m_renderSize, m_projectionBuffer);
		m_startMenu->setScale(m_renderSize);
	});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize.y = (int)f;
		calcOthoProj(m_renderSize, m_projectionBuffer);
		m_startMenu->setScale(m_renderSize);
	});
	calcOthoProj(m_renderSize, m_projectionBuffer);

	// Create main menu
	m_startMenu = std::make_shared<StartMenu>(m_engine);
	m_startMenu->setScale(m_renderSize);
	m_startMenu->addCallback(StartMenu::on_start_game, [&]() {
		m_menuState = on_game;
	});
	m_startMenu->addCallback(StartMenu::on_start_puzzle, [&]() {
		m_menuState = on_puzzle;
	});
	m_startMenu->addCallback(StartMenu::on_options, [&]() {
		m_menuState = on_menu;
	});
	m_startMenu->addCallback(StartMenu::on_quit, [&]() {
		m_menuState = on_exit;
	});
}

void UI_Module::frameTick(const float & deltaTime)
{
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glViewport(0, 0, (GLsizei)m_renderSize.x, (GLsizei)m_renderSize.y);
	glScissor(0, 0, (GLsizei)m_renderSize.x, (GLsizei)m_renderSize.y);
	
	m_projectionBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
	m_startMenu->renderElement(deltaTime, glm::vec2(0.0f), m_renderSize);

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);
	Shader::Release();
}

UI_Module::MenuState UI_Module::getMenuState() const
{
	return m_menuState;
}

void UI_Module::applyMouseEvent(const MouseEvent & mouseEvent)
{
	m_startMenu->mouseAction(mouseEvent);
}

void UI_Module::applyCursorPos(const double & xPos, const double & yPos)
{
	m_mouseEvent.m_xPos = xPos;
	m_mouseEvent.m_yPos = m_renderSize.y - yPos;

	m_startMenu->mouseAction(m_mouseEvent);
}

void UI_Module::applyCursorButton(const int & button, const int & action, const int & mods)
{
	m_mouseEvent.m_button = button;
	m_mouseEvent.m_action = action;
	m_mouseEvent.m_mods = mods;
	m_startMenu->mouseAction(m_mouseEvent);
}

void UI_Module::applyChar(const unsigned int & character)
{
	m_startMenu->keyChar(character);
}

void UI_Module::applyKey(const int & key, const int & scancode, const int & action, const int & mods)
{
	m_startMenu->keyboardAction(key, scancode, action, mods);
}