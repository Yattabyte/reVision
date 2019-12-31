#pragma once
#ifndef UI_DECORATOR_H
#define UI_DECORATOR_H

#include "Modules/UI/Basic Elements/UI_Element.h"


/** A UI decorator object, following the Decorator design pattern. */
class UI_Decorator : public UI_Element {
public:
	// Public (De)Constructors
	/** Construct a decorator, decorating the supplied component.
	@param	engine			the engine to use. 
	@param	component		the component to decorate. */
	UI_Decorator(Engine& engine, const std::shared_ptr<UI_Element>& component) noexcept;


	// Public Interface Implementations
	void renderElement(const float& deltaTime, const glm::vec2& position = glm::vec2(0.0f), const glm::vec2& scale = glm::vec2(1.0f)) override;
	void mouseAction(const MouseEvent& mouseEvent) override;
	void keyboardAction(const KeyboardEvent& keyboardEvent) override;
	void userAction(ActionState& actionState) override;


protected:
	// Protected Attributes
	const std::shared_ptr<UI_Element> m_component;
};

#endif // UI_DECORATOR_H