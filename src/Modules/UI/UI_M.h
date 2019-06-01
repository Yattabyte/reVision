#pragma once
#ifndef UI_MODULE_H
#define UI_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/MouseEvent.h"
#include "Modules/UI/KeyboardEvent.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/ActionState.h"
#include <memory>
#include <vector>


/** A module responsible for the overall user interface. */
class UI_Module : public Engine_Module {
public:
	// Public (de)Constructors
	/** Destroy the UI module. */
	inline ~UI_Module() = default;
	/** Construct a UI module. */
	inline UI_Module() = default;


	// Public Interface Implementation
	virtual void initialize(Engine * engine) override;
	virtual void frameTick(const float & deltaTime) override;


	// Public Methods
	/** Push a new UI element onto a stack to receive input and be rendered. 
	@param	rootElement			the main element of focus for this UI system. */
	void pushRootElement(const std::shared_ptr<UI_Element> & rootElement);
	/** Pop the top UI element off the stack. */
	void popRootElement();
	/** Push a new UI element onto a stack to receive userAction input only.
	@param	focusedElement		a separate element to receive input. */
	void pushFocusedElement(const std::shared_ptr<UI_Element> & focusedElement);
	/** Pop the top focused element off the stack. */
	void popFocusedElement();
	/** Remove the root UI element from the UI system. */
	void clear();
	/** Propagates mouse movement input to all UI elements.
	@param		xPos			the 'x' axis position of the mouse
	@param		yPos			the 'y' axis position of the mouse. */
	void applyCursorPos(const double & xPos, const double & yPos);
	/** Propagates mouse button input to all UI elements.
	@param		button			the mouse button used
	@param		action			the mouse button action (pressed, released, etc)
	@param		mods			any mouse modifiers used. */
	void applyCursorButton(const int & button, const int & action, const int & mods);
	/** Propagates keyboard character input to all UI elements.
	@param		character		the character inputed. */
	void applyChar(const unsigned int & character);
	/** Propagates keyboard key input to all UI elements.
	@param		key				The keyboard key that was pressed or released.
	@param		scancode		The system-specific scancode of the key.
	@param		action			Action::PRESS, RELEASE or REPEAT.
	@param		mods			Bit field describing which modifier keys were held down. */
	void applyKey(const int & key, const int & scancode, const int & action, const int & mods);
	/** Apply an action state to the the current focused UI element.
	@param		actionState		the action state to apply. */
	void applyActionState(ActionState & actionState);
	/** Add a new callback to be called as-soon-as-possible. 
	@param	callback		the callback to call later. */
	void pushCallback(const std::function<void()> & callback);
	

private:
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	glm::ivec2 m_renderSize = glm::ivec2(1);
	StaticBuffer m_projectionBuffer;
	std::vector<std::shared_ptr<UI_Element>> m_rootElement, m_focusedElement;
	std::vector<std::function<void()>> m_callbacks;
	MouseEvent m_mouseEvent;
	KeyboardEvent m_keyboardEvent;
};

#endif // UI_MODULE_H