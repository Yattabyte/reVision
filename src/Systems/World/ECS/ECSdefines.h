#pragma once

#include <cstring>
#include <map>
#include <vector>

typedef std::pair<const char *, unsigned int> ECShandle;

static enum ECS_MESSAGE_COMMAND_TYPES
{
	// SPATIAL
	SET_POSITION,
	SET_ORIENTATION,
	SET_SCALE,
	SET_TRANSFORM,

	// MODEL
	SET_MODEL_DIR,
	SET_MODEL_SKIN,
	SET_MODEL_ANIMATION,

	// LIGHTING
	SET_LIGHT_COLOR,
	SET_LIGHT_INTENSITY,
	SET_LIGHT_RADIUS,
	SET_LIGHT_CUTOFF,

	// Reflectors
	SET_REFLECTOR_RADIUS,
};