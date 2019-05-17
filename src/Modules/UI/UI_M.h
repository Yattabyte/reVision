#pragma once
#ifndef UI_MODULE_H
#define UI_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/MouseEvent.h"
#include "Utilities/GL/StaticBuffer.h"
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
	/** Propagates mouse movement input to all UI elements.
	@param		xPos	the 'x' axis position of the mouse
	@param		yPos	the 'y' axis position of the mouse. */
	void applyCursorPos(const double & xPos, const double & yPos);
	/** Propagates mouse button input to all UI elements.
	@param		button	the mouse button used
	@param		action	the mouse button action (pressed, released, etc)
	@param		mods	any mouse modifiers used. */
	void applyCursorButton(const int & button, const int & action, const int & mods);
	/** Propagates keyboard character input to all UI elements.
	@param		character	the character inputed. */
	void applyChar(const unsigned int & character);
	/** Propagates keyboard key input to all UI elements.
	@param		key			The keyboard key that was pressed or released.
	@param		scancode	The system-specific scancode of the key.
	@param		action		GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
	@param		mods		Bit field describing which modifier keys were held down. */
	void applyKey(const int & key, const int & scancode, const int & action, const int & mods);
	/** Returns whether or not this module requires the mouse.
	@returns	true when active, false otherwise. */
	bool isCursorActive() const;


private:
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	glm::ivec2 m_renderSize = glm::ivec2(1);
	StaticBuffer m_projectionBuffer;
	std::shared_ptr<UI_Element> m_startMenu, m_optionsMenu;
	MouseEvent m_mouseEvent;
};

#endif // UI_MODULE_H