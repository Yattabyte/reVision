#pragma once
#ifndef ACTION_STATE_H
#define ACTION_STATE_H

#include <map>
#include <string>
#include <vector>


/** A container class that holds the action state for the engine, such as forward/back/left/right and amount. */
class ActionState final {
public:
	// Public (De)Constructors
	/** Destroy the action state. */
	inline ~ActionState() = default;
	/** Construct the action state. */
	ActionState() noexcept;


	// Public Static Enumerations
	/** Enumeration for whether the action key was pressed, released, or repeating. */
	enum class State : int {
		RELEASE,
		PRESS,
		REPEAT
	};
	/** Enumeration for indexing into actions. */
	enum class Action : unsigned int {
		MOUSE_X,
		MOUSE_Y,
		MOUSE_L,
		MOUSE_R,
		LOOK_X,
		LOOK_Y,
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		JUMP,
		CROUCH,
		RUN,
		FIRE1,
		FIRE2,
		UI_UP,
		UI_DOWN,
		UI_LEFT,
		UI_RIGHT,
		UI_ENTER,
		UI_ESCAPE,
		ACTION_COUNT
	};


	// Public Static Methods
	/** Retrieve a static list of all input-actions.
	@return	std::vector of action names as strings */
	static std::vector<std::string> Action_Strings() {
		static const std::vector<std::string> actionStrings = {
			"MOUSE_X",
			"MOUSE_Y",
			"MOUSE_L",
			"MOUSE_R",
			"LOOK_X",
			"LOOK_Y",
			"FORWARD",
			"BACKWARD",
			"LEFT",
			"RIGHT",
			"JUMP",
			"CROUCH",
			"RUN",
			"FIRE1",
			"FIRE2",
			"UI_UP",
			"UI_DOWN",
			"UI_LEFT",
			"UI_RIGHT",
			"UI_ENTER",
			"UI_ESCAPE",
		};
		return actionStrings;
	};


	// Public Methods
	/** Retrieve the value for a specific action category.
	@param	index		the action index category to look-up.
	@return				the value reference belonging found at the index. */
	float& operator[](const ActionState::Action& index) noexcept;
	/** Retrieve the value for a specific action category.
	@param	index		the action index category to look-up.
	@return				the value const reference belonging found at the index. */
	const float& operator[](const ActionState::Action& index) const noexcept;
	/** Retrieve the state for a specific action category.
	@param	index		the action index category to look-up.
	@return				the state belonging found at the index, such as pressed, released, or repeating. */
	ActionState::State isAction(const ActionState::Action& actionEnum) noexcept;


private:
	// Protected Attributes
	std::map<ActionState::Action, std::pair<bool, float>> m_keyStates;
};

#endif // ACTION_STATE_H