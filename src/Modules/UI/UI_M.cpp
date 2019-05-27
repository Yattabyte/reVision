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
		if (m_uiElement)
			m_uiElement->setScale(m_renderSize);
	});
	preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_renderSize.y = (int)f;
		calcOthoProj(m_renderSize, m_projectionBuffer);
		if (m_uiElement)
			m_uiElement->setScale(m_renderSize);
	});
	calcOthoProj(m_renderSize, m_projectionBuffer);	
}

void UI_Module::frameTick(const float & deltaTime)
{
	if (m_uiElement) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_SCISSOR_TEST);
		glViewport(0, 0, (GLsizei)m_renderSize.x, (GLsizei)m_renderSize.y);
		glScissor(0, 0, (GLsizei)m_renderSize.x, (GLsizei)m_renderSize.y);

		m_projectionBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 2);
		m_uiElement->renderElement(deltaTime, glm::vec2(0.0f), m_renderSize);

		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_BLEND);
		Shader::Release();
	}
}

void UI_Module::setRootElement(const std::shared_ptr<UI_Element>& rootElement)
{
	m_uiElement = rootElement;
	m_uiElement->setScale(m_renderSize);
}

void UI_Module::clearRootElement()
{
	if (m_uiElement)
		m_uiElement.reset();
}

void UI_Module::applyMouseEvent(const MouseEvent & mouseEvent)
{
	if (m_uiElement)
		m_uiElement->mouseAction(mouseEvent);
}

void UI_Module::applyChar(const unsigned int & character)
{
	if (m_uiElement)
		m_uiElement->keyChar(character);
}

void UI_Module::applyKey(const int & key, const int & scancode, const int & action, const int & mods)
{
	if (m_uiElement)
		m_uiElement->keyboardAction(key, scancode, action, mods);
}