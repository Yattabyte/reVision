#pragma once
#ifndef UI_MODULE_H
#define UI_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/MouseEvent.h"
#include <memory>
#include <vector>


/** A module responsible for the overall user interface. */
class UI_Module : public Engine_Module {
public:
	// (de)Constructors
	~UI_Module() = default;
	UI_Module() = default;


	// Public Interface Implementation
	/** Initialize the module. */
	virtual void initialize(Engine * engine) override;
	/** Tick the ui by a frame. 
	@param	deltaTime	the amount of time passed since last frame. */
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
	/** Returns whether or not this module requires the mouse.
	@returns	true when active, false otherwise. */
	bool isCursorActive() const;


private:
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::vector<std::shared_ptr<UI_Element>> m_uiElements;
	std::shared_ptr<UI_Element> m_startMenu;
	MouseEvent m_mouseEvent;
};

#endif // UI_MODULE_H