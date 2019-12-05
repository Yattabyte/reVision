#include "Modules/UI/Decorators/UI_Decorator.h"


UI_Decorator::UI_Decorator(Engine& engine, const std::shared_ptr<UI_Element>& component) noexcept :
	UI_Element(engine), 
	m_component(component)
{
}

void UI_Decorator::renderElement(const float& deltaTime, const glm::vec2& position, const glm::vec2& scale) noexcept 
{
	UI_Element::renderElement(deltaTime, position, scale);
	const glm::vec2 newPosition = position + m_position;
	const glm::vec2 newScale = glm::min(m_scale, scale);
	m_component->renderElement(deltaTime, newPosition, newScale);
}

void UI_Decorator::mouseAction(const MouseEvent& mouseEvent) noexcept 
{
	UI_Element::mouseAction(mouseEvent);
	if (getVisible() && getEnabled() && mouseWithin(mouseEvent)) {
		MouseEvent subEvent = mouseEvent;
		subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
		subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
		m_component->mouseAction(subEvent);
	}
	else
		m_component->clearFocus();
}

void UI_Decorator::keyboardAction(const KeyboardEvent& keyboardEvent) noexcept
{
	m_component->keyboardAction(keyboardEvent);
	UI_Element::keyboardAction(keyboardEvent);
}

void UI_Decorator::userAction(ActionState& actionState) noexcept
{
	m_component->userAction(actionState);
	UI_Element::userAction(actionState);
}