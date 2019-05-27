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
	/** Place a root UI element to receive input and be rendered. 
	@param	rootElement		the main element of focus for this UI system. */
	void setRootElement(const std::shared_ptr<UI_Element> & rootElement);
	/** Remove the root UI element from the UI system. */
	void clearRootElement();
	/** Propagates a mouse event to all UI elements. 
	@param	mouseEvent	the mouse event to apply. */
	void applyMouseEvent(const MouseEvent & mouseEvent);	
	/** Propagates keyboard character input to all UI elements.
	@param		character	the character inputed. */
	void applyChar(const unsigned int & character);
	/** Propagates keyboard key input to all UI elements.
	@param		key			The keyboard key that was pressed or released.
	@param		scancode	The system-specific scancode of the key.
	@param		action		GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
	@param		mods		Bit field describing which modifier keys were held down. */
	void applyKey(const int & key, const int & scancode, const int & action, const int & mods);


private:
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	glm::ivec2 m_renderSize = glm::ivec2(1);
	StaticBuffer m_projectionBuffer;
	std::shared_ptr<UI_Element> m_uiElement;
	MouseEvent m_mouseEvent;
};

#endif // UI_MODULE_H