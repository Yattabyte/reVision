#include "Modules/UI/UI_M.h"
#include "Engine.h"


UI_Module::UI_Module(Engine& engine) : 
	Engine_Module(engine)
{
}

void UI_Module::initialize()
{
	Engine_Module::initialize();
	m_engine.getManager_Messages().statement("Loading Module: User Interface...");

	// Preferences
	auto& preferences = m_engine.getPreferenceState();
	constexpr static auto calcOthoProj = [](const glm::ivec2& renderSize, StaticBuffer& projectionBuffer) {
		const glm::mat4 proj = glm::ortho<float>(0.0f, static_cast<float>(renderSize.x), 0.0f, static_cast<float>(renderSize.y), -1.0f, 1.0f);
		projectionBuffer.write(0, sizeof(glm::mat4), &proj[0][0]);
	};
	m_projectionBuffer = StaticBuffer(sizeof(glm::mat4), nullptr, GL_DYNAMIC_STORAGE_BIT);
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::Preference::C_WINDOW_HEIGHT, m_renderSize.y);
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) {
		m_renderSize.x = static_cast<int>(f);
		calcOthoProj(m_renderSize, m_projectionBuffer);
		for (const auto& element : m_rootElement)
			element->setScale(m_renderSize);
		});
	preferences.addCallback(PreferenceState::Preference::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) {
		m_renderSize.y = static_cast<int>(f);
		calcOthoProj(m_renderSize, m_projectionBuffer);
		for (const auto& element : m_rootElement)
			element->setScale(m_renderSize);
		});
	calcOthoProj(m_renderSize, m_projectionBuffer);
}

void UI_Module::deinitialize()
{
	m_engine.getManager_Messages().statement("Unloading Module: User Interface...");

	// Update indicator
	*m_aliveIndicator = false;

	m_rootElement.clear();
}

void UI_Module::frameTick(const float& deltaTime)
{
	glViewport(0, 0, static_cast<GLsizei>(m_renderSize.x), static_cast<GLsizei>(m_renderSize.y));
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Copy the list of callbacks, execute a copy of them
	// We use a copy because any callback may alter the list,
	auto copySelection = m_callbacks;
	m_callbacks.clear();
	for (const auto& func : copySelection)
		func();

	if (!m_rootElement.empty() && m_rootElement.back()) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		m_projectionBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
		m_rootElement.back()->renderElement(deltaTime, glm::vec2(0.0F), m_renderSize);

		glDisable(GL_BLEND);
		Shader::Release();
	}
}

void UI_Module::pushRootElement(const std::shared_ptr<UI_Element>& rootElement)
{
	m_rootElement.push_back(rootElement);
	if (rootElement)
		rootElement->setScale(m_renderSize);
}

void UI_Module::popRootElement() noexcept
{
	if (m_rootElement.size() > 1)
		m_rootElement.pop_back();
}

void UI_Module::setFocusMap(const std::shared_ptr<FocusMap>& focusMap) noexcept
{
	m_focusMap = focusMap;
}

std::shared_ptr<FocusMap> UI_Module::getFocusMap() const noexcept
{
	return m_focusMap;
}

void UI_Module::clear() noexcept
{
	m_rootElement.clear();
	m_focusMap.reset();
}

void UI_Module::applyCursorPos(const double& xPos, const double& yPos)
{
	m_mouseEvent.m_xPos = xPos;
	m_mouseEvent.m_yPos = m_renderSize.y - yPos;
	m_mouseEvent.m_action = MouseEvent::Action::MOVE;

	if (!m_rootElement.empty())
		m_rootElement.back()->mouseAction(m_mouseEvent);
}

void UI_Module::applyCursorButton(const int& button, const int& action, const int& mods)
{
	m_mouseEvent.m_button = MouseEvent::Key(button);
	m_mouseEvent.m_action = MouseEvent::Action(action);
	m_mouseEvent.m_mods = mods;

	if (!m_rootElement.empty())
		m_rootElement.back()->mouseAction(m_mouseEvent);
}

void UI_Module::applyChar(const unsigned int& character)
{
	m_keyboardEvent.setChar(character);
	if (!m_rootElement.empty())
		m_rootElement.back()->keyboardAction(m_keyboardEvent);
	m_keyboardEvent.setChar(0);
}

void UI_Module::applyKey(const int& key, const int& /*unused*/, const int& action, const int& /*unused*/)
{
	m_keyboardEvent.setState(KeyboardEvent::Key(static_cast<unsigned int>(key)), KeyboardEvent::Action(action));
	if (!m_rootElement.empty())
		m_rootElement.back()->keyboardAction(m_keyboardEvent);
}

void UI_Module::applyActionState(ActionState& actionState)
{
	if (m_focusMap)
		m_focusMap->applyActionState(actionState);
}

void UI_Module::pushCallback(const std::function<void()>& callback)
{
	m_callbacks.push_back(callback);
}