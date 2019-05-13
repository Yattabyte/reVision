#include "Modules/UI/UI_M.h"
#include "Modules/UI/Macro Elements/StartMenu.h"
#include "Modules/UI/Macro Elements/OptionsMenu.h"
#include "Engine.h"


void UI_Module::initialize(Engine * engine)
{
	// Preferences
	auto & preferences = engine->getPreferenceState();
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
		m_startMenu->setPosition(glm::vec2(m_renderSize) / 2.0f);
		m_optionsMenu->setPosition(glm::vec2(m_renderSize) / 2.0f);
	});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize.y = (int)f;
		calcOthoProj(m_renderSize, m_projectionBuffer);
		m_startMenu->setPosition(glm::vec2(m_renderSize) / 2.0f);
		m_optionsMenu->setPosition(glm::vec2(m_renderSize) / 2.0f);
	});
	calcOthoProj(m_renderSize, m_projectionBuffer);

	// Create main menu
	m_startMenu = std::make_shared<StartMenu>(engine);
	m_startMenu->setPosition(glm::vec2(m_renderSize) / 2.0f);
	m_startMenu->addCallback(StartMenu::on_start, [&, engine]() { engine->getModule_Game().startGame(); m_startMenu->setVisible(false); });
	m_startMenu->addCallback(StartMenu::on_options, [&, engine]() { m_optionsMenu->setVisible(true); m_startMenu->setVisible(false); });
	m_startMenu->addCallback(StartMenu::on_quit, [engine]() {engine->shutDown(); });

	// Create options menu
	m_optionsMenu = std::make_shared<OptionsMenu>(engine);
	m_optionsMenu->setPosition(glm::vec2(m_renderSize) / 2.0f);
	m_optionsMenu->addCallback(OptionsMenu::on_back, [&]() { m_startMenu->setVisible(true); m_optionsMenu->setVisible(false); });
	m_optionsMenu->setVisible(false);
	m_uiElements = { m_startMenu, m_optionsMenu };
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
	for each (auto & element in m_uiElements)
		element->renderElement(deltaTime, glm::vec2(0.0f), m_renderSize);

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);
	Shader::Release();
}

void UI_Module::applyCursorPos(const double & xPos, const double & yPos)
{
	m_mouseEvent.m_xPos = xPos;
	m_mouseEvent.m_yPos = m_renderSize.y - yPos;

	for each (auto & element in m_uiElements)
		element->mouseAction(m_mouseEvent);			
}

void UI_Module::applyCursorButton(const int & button, const int & action, const int & mods)
{
	m_mouseEvent.m_button = button;
	m_mouseEvent.m_action = action;
	m_mouseEvent.m_mods = mods;
	for each (auto & element in m_uiElements)
		if (element->mouseAction(m_mouseEvent))
			break;
}

void UI_Module::applyChar(const unsigned int & character)
{
	for each (auto & element in m_uiElements)
		element->keyChar(character);
}

void UI_Module::applyKey(const int & key, const int & scancode, const int & action, const int & mods)
{
	for each (auto & element in m_uiElements)
		element->keyboardAction(key, scancode, action, mods);
}

bool UI_Module::isCursorActive() const
{
	return true;
}