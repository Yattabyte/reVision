#pragma once

static enum ECS_MESSAGE_COMMAND_TYPES
{
	// MODEL
	SET_MODEL_DIR,
	SET_MODEL_TRANSFORM,
	SET_MODEL_SKIN,
	PLAY_ANIMATION,

	// LIGHTING
	SET_LIGHT_COLOR,
	SET_LIGHT_INTENSITY,
	SET_LIGHT_ORIENTATION,
	SET_LIGHT_TRANSFORM,
};