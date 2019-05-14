#pragma once
#ifndef UI_DECORATOR_H
#define UI_DECORATOR_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include <memory>


/** A UI decorator object, following the Decorator design pattern. */
class UI_Decorator : public UI_Element
{
public:
	// Public (de)Constructors
	inline ~UI_Decorator() = default;
	inline UI_Decorator(const std::shared_ptr<UI_Element> & component) : m_component(component) {}


	// Public Interface Implementations
	inline virtual bool mouseAction(const MouseEvent & mouseEvent) override {
		if (!getVisible() || !getEnabled()) return false;
		if (mouseWithin(mouseEvent)) {
			MouseEvent subEvent = mouseEvent;
			subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
			subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
			bool consumed = false;
			for each (auto & child in m_children) {
				if (child->mouseAction(subEvent)) {
					consumed = true;
					break;
				}
			}
			if (!consumed)
				consumed = m_component->mouseAction(subEvent);
			enactCallback(on_mouse_enter);
			if (mouseEvent.m_action == MouseEvent::PRESS)
				enactCallback(on_mouse_press);
			else
				enactCallback(on_mouse_release);
			return consumed;
		}
		enactCallback(on_mouse_exit);
		return false;
	}
	inline virtual void keyChar(const unsigned int & character) override {
		UI_Element::keyChar(character);
		m_component->keyChar(character);
	}
	inline virtual void keyboardAction(const int & key, const int & scancode, const int & action, const int & mods) override {
		UI_Element::keyboardAction(key, scancode, action, mods);
		m_component->keyboardAction(key, scancode, action, mods);
	}
	inline virtual void renderElement(const float & deltaTime, const glm::vec2 & position = glm::vec2(0.0f), const glm::vec2 & scale = glm::vec2(1.0f)) override {
		UI_Element::renderElement(deltaTime, position, scale);
		const glm::vec2 newPosition = position + m_position;
		const glm::vec2 newScale = glm::min(m_scale, scale);
		glScissor(
			GLint(newPosition.x - (newScale.x)),
			GLint(newPosition.y - (newScale.y)),
			GLsizei(newScale.x * 2.0f),
			GLsizei(newScale.y * 2.0f)
		);
		m_component->renderElement(deltaTime, newPosition, newScale);
	}


protected:
	// Protected Attributes
	const std::shared_ptr<UI_Element> m_component;
};

#endif // UI_DECORATOR_H