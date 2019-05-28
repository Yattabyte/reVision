#pragma once
#ifndef ACTION_STATE_H
#define ACTION_STATE_H

#include <map>
#include <vector>


/** A container class that holds the action state for the engine, such as forward/back/left/right and amount. */
class ActionState : public std::map<unsigned int, float> {
public:
	// Public (de)Constructors
	/** Destroy the action state. */
	inline ~ActionState() = default;
	/** Construct the action state. */
	inline ActionState() {
		for (unsigned int x = 0; x < ACTION_COUNT; ++x)
			insert(std::pair<unsigned int, float>(x, 0.0f));
	}


	// Public Static Enumerations
	/** Enumeration for indexing into actions. */
	const enum ACTION_ENUM {
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
			"UI_UP",
			"UI_DOWN",
			"UI_LEFT",
			"UI_RIGHT",
			"UI_ENTER",
			"UI_ESCAPE",
		};
		return actionStrings;
	};	

	const enum Test : char {
		A, 
		B,
		C,
	};


	// Public Methods
	inline bool isAction(const ActionState::ACTION_ENUM && actionEnum) {
		if (at(actionEnum) > 0.5f) {
			if (!m_keyPressStates[actionEnum]) {
				m_keyPressStates[actionEnum] = true;
				return true;
			}
		}
		else
			m_keyPressStates[actionEnum] = false;
		return false;
	}


private:
	// Private Attributes
	std::map<ActionState::ACTION_ENUM, bool> m_keyPressStates;
};

#endif // ACTION_STATE_H