#pragma once
#ifndef UI_MODULE_H
#define UI_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Modules/UI/FocusMap.h"
#include "Modules/UI/MouseEvent.h"
#include "Modules/UI/KeyboardEvent.h"
#include "Utilities/ActionState.h"
#include "Utilities/GL/StaticBuffer.h"
#include <vector>


/** A module responsible for the overall user interface. */
class UI_Module final : public Engine_Module {
public:
	// Public (De)Constructors
	/** Destroy the UI module. */
	inline ~UI_Module() noexcept = default;
	/** Construct a UI module.
	@param	engine				reference to the engine to use. */
	explicit UI_Module(Engine& engine);


	// Public Interface Implementations
	void initialize() final;
	void deinitialize() final;


	// Public Methods
	/** Tick this module by a specific amount of delta time.
	@param	deltaTime			the amount of time since last frame. */
	void frameTick(const float& deltaTime);
	/** Push a new UI element onto a stack to receive input and be rendered.
	@param	rootElement			the main element of focus for this UI system. */
	void pushRootElement(const std::shared_ptr<UI_Element>& rootElement);
	/** Pop the top UI element off the stack. */
	void popRootElement() noexcept;
	/** Set a new focus map to be used for user action input.
	@param	focusMap			the new focus map to use. */
	void setFocusMap(const std::shared_ptr<FocusMap>& focusMap) noexcept;
	/** Retrieve the active focus map, used for user input.
	@return						the active focus map. */
	std::shared_ptr<FocusMap> getFocusMap() const noexcept;
	/** Remove the root UI element from the UI system. */
	void clear() noexcept;
	/** Propagates mouse movement input to all UI elements.
	@param	xPos				the 'x' axis position of the mouse
	@param	yPos				the 'y' axis position of the mouse. */
	void applyCursorPos(const double& xPos, const double& yPos);
	/** Propagates mouse button input to all UI elements.
	@param	button				the mouse button used
	@param	action				the mouse button action (pressed, released, etc)
	@param	mods				any mouse modifiers used. */
	void applyCursorButton(const int& button, const int& action, const int& mods);
	/** Propagates keyboard character input to all UI elements.
	@param	character			the character inputed. */
	void applyChar(const unsigned int& character);
	/** Propagates keyboard key input to all UI elements.
	@param	key					the keyboard key that was pressed or released.
	@param	scan-code			the system-specific scan-code of the key.
	@param	action				PRESS, RELEASE or REPEAT.
	@param	mods				bit field describing which modifier keys were held down. */
	void applyKey(const int& key, const int& scancode, const int& action, const int& mods);
	/** Apply an action state to the current focused UI element.
	@param	actionState			the action state to apply. */
	void applyActionState(ActionState& actionState) noexcept;
	/** Add a new callback to be called as-soon-as-possible.
	@param	callback			the callback to call later. */
	void pushCallback(const std::function<void()>& callback);


private:
	// Private and deleted
	/** Disallow module move constructor. */
	inline UI_Module(UI_Module&&) noexcept = delete;
	/** Disallow module copy constructor. */
	inline UI_Module(const UI_Module&) noexcept = delete;
	/** Disallow module move assignment. */
	inline const UI_Module& operator =(UI_Module&&) noexcept = delete;
	/** Disallow module copy assignment. */
	inline const UI_Module& operator =(const UI_Module&) noexcept = delete;


	// Private Attributes
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	glm::ivec2 m_renderSize = glm::ivec2(1);
	StaticBuffer m_projectionBuffer;
	std::vector<std::shared_ptr<UI_Element>> m_rootElement;
	std::vector<std::function<void()>> m_callbacks;
	std::shared_ptr<FocusMap> m_focusMap;
	MouseEvent m_mouseEvent;
	KeyboardEvent m_keyboardEvent;
};

#endif // UI_MODULE_H