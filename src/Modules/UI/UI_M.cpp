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
		for each (auto element in m_rootElement)
			element->setScale(m_renderSize);
	});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize.y = (int)f;
		calcOthoProj(m_renderSize, m_projectionBuffer);
		for each (auto element in m_rootElement)
			element->setScale(m_renderSize);
	});
	calcOthoProj(m_renderSize, m_projectionBuffer);	
}

void UI_Module::frameTick(const float & deltaTime)
{
	if (m_rootElement.size() && m_rootElement.back()) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, (GLsizei)m_renderSize.x, (GLsizei)m_renderSize.y);

		m_projectionBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
		m_rootElement.back()->renderElement(deltaTime, glm::vec2(0.0f), m_renderSize);

		glDisable(GL_BLEND);
		Shader::Release();
	}
}

void UI_Module::pushRootElement(const std::shared_ptr<UI_Element>& rootElement)
{
	m_rootElement.push_back(rootElement);
	if (rootElement)
		rootElement->setScale(m_renderSize);

	// Clear focus stack, set it to root
	m_focusedElement = m_rootElement;
}

void UI_Module::popRootElement()
{
	if (m_rootElement.size() > 1)
		m_rootElement.pop_back();

	// Clear focus stack, set it to root
	m_focusedElement = m_rootElement;
}

void UI_Module::pushFocusedElement(const std::shared_ptr<UI_Element>& focusedElement)
{
	m_focusedElement.push_back(focusedElement);
}

void UI_Module::popFocusedElement()
{
	if (m_focusedElement.size() > 1)
		m_focusedElement.pop_back();
}

void UI_Module::clear()
{
	m_rootElement.clear();
	m_focusedElement.clear();
}

void UI_Module::applyCursorPos(const double & xPos, const double & yPos)
{
	m_mouseEvent.m_xPos = xPos;
	m_mouseEvent.m_yPos = m_renderSize.y - yPos;
	m_mouseEvent.m_action = MouseEvent::MOVE;

	if (m_rootElement.size())
		m_rootElement.back()->mouseAction(m_mouseEvent);
}

void UI_Module::applyCursorButton(const int & button, const int & action, const int & mods)
{
	m_mouseEvent.m_button = button;
	m_mouseEvent.m_action = action;
	m_mouseEvent.m_mods = mods;

	if (m_rootElement.size())
		m_rootElement.back()->mouseAction(m_mouseEvent);
}

void UI_Module::applyChar(const unsigned int & character)
{
	m_keyboardEvent.setChar(character);
	if (m_rootElement.size())
		m_rootElement.back()->keyboardAction(m_keyboardEvent);
	m_keyboardEvent.setChar(0);
}

void UI_Module::applyKey(const int & key, const int & scancode, const int & action, const int & mods)
{
	m_keyboardEvent.setState(KeyboardEvent::Key((unsigned int)key), KeyboardEvent::Action(action));
	if (m_rootElement.size())
		m_rootElement.back()->keyboardAction(m_keyboardEvent);
}

void UI_Module::applyActionState(ActionState & actionState)
{
	if (m_focusedElement.size())
		m_focusedElement.back()->userAction(actionState);
}
