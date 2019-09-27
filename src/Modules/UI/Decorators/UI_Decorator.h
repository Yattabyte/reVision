#pragma once
#ifndef UI_DECORATOR_H
#define UI_DECORATOR_H

#include "Modules/UI/Basic Elements/UI_Element.h"


/** A UI decorator object, following the Decorator design pattern. */
class UI_Decorator : public UI_Element {
public:
	// Public (de)Constructors
	/** Destroy this decorator. */
	inline ~UI_Decorator() = default;
	/** Construct a decorator, decorating the supplied component.
	@param	component		the component to decorate. */
	inline UI_Decorator(Engine* engine, const std::shared_ptr<UI_Element>& component)
		: UI_Element(engine), m_component(component) {}


	// Public Interface Implementations
	inline virtual void renderElement(const float& deltaTime, const glm::vec2& position = glm::vec2(0.0f), const glm::vec2& scale = glm::vec2(1.0f)) override {
		UI_Element::renderElement(deltaTime, position, scale);
		const glm::vec2 newPosition = position + m_position;
		const glm::vec2 newScale = glm::min(m_scale, scale);
		m_component->renderElement(deltaTime, newPosition, newScale);
	}
	inline virtual void mouseAction(const MouseEvent& mouseEvent) override {
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
	inline virtual void keyboardAction(const KeyboardEvent& keyboardEvent) override {
		m_component->keyboardAction(keyboardEvent);
		UI_Element::keyboardAction(keyboardEvent);
	}
	inline virtual void userAction(ActionState& actionState) override {
		m_component->userAction(actionState);
		UI_Element::userAction(actionState);
	}


protected:
	// Protected Attributes
	const std::shared_ptr<UI_Element> m_component;
};

#endif // UI_DECORATOR_H