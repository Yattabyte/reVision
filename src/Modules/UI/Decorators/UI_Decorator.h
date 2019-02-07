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
	~UI_Decorator() = default;
	UI_Decorator(const std::shared_ptr<UI_Element> & component) : m_component(component) {}


	// Public Interface Implementations
	virtual void setDepth(const float & depth) override {
		UI_Element::setDepth(depth + 1.0f);
		m_component->setDepth(depth);
	}
	virtual bool doElementsExceedBounds(const glm::vec2 & scale, const glm::vec2 & positionOffset = glm::vec2(0.0f)) const override {
		if (m_component->doElementsExceedBounds(scale, positionOffset + m_component->getPosition()))
			return true;
		const auto childPos = m_component->getPosition();
		const auto childScl = m_component->getScale();
		if (
			((childPos.x + positionOffset.x) - childScl.x) < (-scale.x) ||
			((childPos.x + positionOffset.x) + childScl.x) > (scale.x) ||
			((childPos.y + positionOffset.y) - childScl.y) < (-scale.y) ||
			((childPos.y + positionOffset.y) + childScl.y) > (scale.y)
			)
			return true;			
		
		return UI_Element::doElementsExceedBounds(scale, positionOffset);
	}
	virtual void mouseMove(const MouseEvent & mouseEvent) override {
		if (!getVisible() || !getEnabled()) return;
		if (mouseWithin(mouseEvent) || doElementsExceedBounds(m_scale)) {
			MouseEvent subEvent = mouseEvent;
			subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
			subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
			for each (auto & child in m_children)
				child->mouseMove(subEvent);
			m_component->mouseMove(subEvent);
			enactCallback(on_mouse_enter);
		}
		else
			enactCallback(on_mouse_exit);
	}
	virtual bool mouseButton(const MouseEvent & mouseEvent) override {
		if (!getVisible() || !getEnabled()) return false;
		if (mouseWithin(mouseEvent) || doElementsExceedBounds(m_scale)) {
			MouseEvent subEvent = mouseEvent;
			subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
			subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
			bool consumed = false;
			for each (auto & child in m_children) {
				if (child->mouseButton(subEvent)) {
					consumed = true;
					break;
				}
			}
			if (!consumed)
				consumed = m_component->mouseButton(subEvent);
			if (mouseEvent.m_button == 0) {
				if (mouseEvent.m_action == 1)
					enactCallback(on_mouse_press);
				else
					enactCallback(on_mouse_release);
			}
			return true;
		}
		return false;
	}
	virtual void keyChar(const unsigned int & character) override {
		UI_Element::keyChar(character);
		m_component->keyChar(character);
	}
	virtual void keyboardAction(const int & key, const int & scancode, const int & action, const int & mods) override {
		UI_Element::keyboardAction(key, scancode, action, mods);
		m_component->keyboardAction(key, scancode, action, mods);
	}
	virtual void renderElement(const float & deltaTime, const glm::vec2 & position = glm::vec2(0.0f), const glm::vec2 & scale = glm::vec2(1.0f)) override {
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