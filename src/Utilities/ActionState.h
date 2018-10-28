#pragma once
#ifndef ACTION_STATE_H
#define ACTION_STATE_H

#include <map>
#include <vector>


/** A container class that holds the action state for the engine, such as forward/back/left/right and amount. */
class ActionState : public std::map<unsigned int, float> {
public:
	// (de)Constructors
	/** Destroy the action state. */
	~ActionState() = default;
	/** Construct the action state. */
	ActionState() {
		for (unsigned int x = 0; x < ACTION_COUNT; ++x)
			insert(std::pair<unsigned int, float>(x, 0.0f));
	}


	// Public Static Enumerations
	/** Enumeration for indexing into actions. */
	const enum ACTION_ENUM {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		JUMP,
		CROUCH,
		RUN,
		LOOK_X,
		LOOK_Y,
		ACTION_COUNT
	};


	// Public Static Methods
	/** Retrieve a static list of all input-actions.
	@return	std::vector of action names as strings */
	static std::vector<std::string> Action_Strings() {
		static const std::vector<std::string> actionStrings = {
			"FORWARD",
			"BACKWARD",
			"LEFT",
			"RIGHT",
			"JUMP",
			"CROUCH",
			"RUN",
			"LOOK_X",
			"LOOK_Y"
		};
		return actionStrings;
	};	
};

#endif // ACTION_STATE_H