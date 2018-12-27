#pragma once
#ifndef COMMON_DEFINITIONS_H
#define COMMON_DEFINITIONS_H 

#include "glm\glm.hpp"


constexpr int	BOARD_WIDTH					= 6u;
constexpr int	BOARD_HEIGHT				= 12u;
constexpr int	TILE_SIZE					= 128;
constexpr int	TickCount_IntroPowerOn		= 150;
constexpr int	TickCount_IntroCountDown	= 300;
constexpr int	TickCount_IntroSum			= TickCount_IntroPowerOn + TickCount_IntroCountDown;
constexpr float TickCount_GameAnimation		= 750.0f;
constexpr int	TickCount_NewLine			= 500u;
constexpr int	TickCount_Time				= 100;
constexpr float TickCount_TileDrop			= 10.0F;
constexpr float TickCount_TileBounce		= 15.0F;
constexpr int	TickCount_Scoring			= 50;
constexpr int	TickCount_Popping			= 25;
constexpr int	TickCount_LevelUp			= 150;

/** OpenGL buffer for boards. */
struct GameBuffer {
	unsigned int types[12 * 6];
	float gravityOffsets[12 * 6];
	float lifeLinear[12 * 6];
	float lanes[6]; glm::vec2 pad1;
	glm::vec3 colorScheme = glm::vec3(0.0f); float pad2;
	glm::ivec2 playerCoords = glm::ivec2(0, 0);
	float heightOffset = 0.0f;
	float sysTime = 0.0f;
	float gameWave = 0.0f;
	float excitementLinear = 0.0f;
	float shakeLinear = 0.0f;
	float scoreAnimLinear = 0.0f;
	float timeAnimLinear = 1.0f;
	int score = 0;
	int highlightIndex = -1;
	int multiplier = 0;
	int stopTimer = -1;
	int gameTimer = 0;
	float nearingTop = 0;
	struct IntroStruct {
		float powerOn = 0.0f;
		float powerSecondary = 0.0f;
		int countDown = -1;
	} intro;
};

#endif // COMMON_DEFINITIONS_H