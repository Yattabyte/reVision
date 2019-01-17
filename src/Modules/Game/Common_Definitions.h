#pragma once
#ifndef COMMON_DEFINITIONS_H
#define COMMON_DEFINITIONS_H 

#include "glm/glm.hpp"


constexpr int	BOARD_WIDTH					= 6u;
constexpr int	BOARD_HEIGHT				= 12u;
constexpr int	TILE_SIZE					= 128;
constexpr float Tile_Gravity				= 0.15f;
constexpr float Tile_DropDuration			= 0.1f;
constexpr float Tile_BounceDuration			= 0.2f;
constexpr float Tile_SwapDuration			= 0.125f;
constexpr float Game_LevelUpDuration		= 1.0f;


/** OpenGL buffer for boards. */
struct GameBuffer {
	struct TileStruct {
		unsigned int type;
		float xOffset;
		float yOffset;
		float lifeLinear;
	} tiles[12 * 6];
	float lanes[6]; glm::vec2 pad1;
	glm::vec3 colorScheme = glm::vec3(0.0f); float pad2;
	glm::ivec2 playerCoords = glm::ivec2(0, 0);
	float heightOffset = 0.0f;
	float sysTime = 0.0f;
	float shakeLinear = 0.0f;
	float scoreAnimLinear = 0.0f;
	float timeAnimLinear = 1.0f;
	int score = 0;
	int highlightIndex = -1;
	int multiplier = 0;
	float stopTimer = -1.0f;
	float gameTimer = 0;
	float nearingTop = 0;
	struct MusicStruct {
		float beat = 0.0f;
	} music;
	struct IntroStruct {
		float powerOn = 0.0f;
		float powerSecondary = 0.0f;
		int countDown = -1;
	} intro;
};

#endif // COMMON_DEFINITIONS_H