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
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		JUMP,
		CROUCH,
		RUN,
		ENTER,
		PAUSE,
		LOOK_X,
		LOOK_Y,
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
			"FORWARD",
			"BACKWARD",
			"LEFT",
			"RIGHT",
			"JUMP",
			"CROUCH",
			"RUN",
			"ENTER",
			"PAUSE",
			"LOOK_X",
			"LOOK_Y"
		};
		return actionStrings;
	};	
};

#endif // ACTION_STATE_H