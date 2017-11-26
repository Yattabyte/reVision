/*
	Action_State

	- A container class that holds the action state for the engine, such as forward/back/left/right and amount
*/



#pragma once
#ifndef ACTION_STATE
#define ACTION_STATE
#ifdef	ENGINE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include <map>
#include <vector>

using namespace std;

static const enum ACTION_ENUM {
	FORWARD,
	BACK,
	LEFT,
	RIGHT,
	JUMP,
	CROUCH,
	RUN,
	ACTION_COUNT
};

static const vector<string> ACTION_STRINGS = {
	"FORWARD",
	"BACK",
	"LEFT",
	"RIGHT",
	"JUMP",
	"CROUCH",
	"RUN"
};

class Action_State : public map<unsigned int, float>
{
public:
	~Action_State() {}
	Action_State() {
		for (unsigned int x = 0; x < ACTION_COUNT; ++x)
			insert(pair<unsigned int, float>(x, 0.0f));
	}
};

#endif // ACTION_STATE