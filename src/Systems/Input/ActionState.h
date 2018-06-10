#pragma once
#ifndef ACTION_STATE_H
#define ACTION_STATE_H

#include <map>
#include <vector>

using namespace std;


/**
 * A container class that holds the action state for the engine, such as forward/back/left/right and amount
 **/
class ActionState : public map<unsigned int, float>
{
public:
	// (de)Constructors
	/** Destroy the action state. */
	~ActionState() {}
	/** Construct the action state. */
	ActionState() {
		for (unsigned int x = 0; x < ACTION_COUNT; ++x)
			insert(pair<unsigned int, float>(x, 0.0f));
	}


	// Public Static Enumerations
	/** Enumeration for indexing into actions. */
	static const enum ACTION_ENUM {
		FORWARD,
		BACK,
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
	/* Retrieve a static list of all input-actions.
	 * @return	vector of action names as strings */
	static vector<string> Action_Strings() {
		static const vector<string> actionStrings = {
			"FORWARD",
			"BACK",
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