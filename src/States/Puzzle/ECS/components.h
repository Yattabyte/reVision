#pragma once
#ifndef PUZZLE_COMPONENTS_H
#define PUZZLE_COMPONENTS_H

#include "Modules/World/ECS/ecsComponent.h"
#include "States/Puzzle/Common_Definitions.h"
#include "Utilities/GL/VectorBuffer.h"
#include "glm/glm.hpp"
#include <deque>


/** Holds Tile State. */
struct TileState {
	// Enumerations
	enum TileType : int {
		A, B, C, D, E,
		NONE,
	} m_type = NONE;
	enum ScoreType : unsigned int {
		UNMATCHED, MATCHED, SCORED
	} m_scoreType = UNMATCHED;
	TileState(const TileType & t = NONE) : m_type(t) {};
};
/** A component representing a basic player. */
struct Board_Component : public ECSComponent<Board_Component> {
	GameBuffer * m_data = nullptr;
	TileState m_tiles[12][6];
	struct TileDropData {
		enum DropState {
			STATIONARY, FALLING, BOUNCING
		} dropState = STATIONARY;
		unsigned int endIndex = 0;
		float delta = 0.0f;
		float time = 0.0f;
		float velocity = 0.0f;
		unsigned int weight = 1;
	} m_tileDrops[12][6];
	struct GamePlayer {
		int xPos = 0;
		int yPos = 0;
		struct TileSwaps {
			int xIndices[2] = { -1,-1 };
			int yIndex = -1;
			float time = 0.0f;
		};
		std::deque<TileSwaps> tileSwaps;
	} m_player;
	int m_rowsToAdd = 0;
	bool m_critical = false;
	bool m_stop = false;
	bool m_skipWaiting = false;
	bool m_gameInProgress = false;
	bool m_gameEnded = false;
	float m_rowClimbTime = 0.0f;
	float m_speed = 1.0F;
	struct GameMusic {
		float accumulator = 0.0f;
		float beatSeconds = 0.0f;
		bool beat = false;
	} m_music;
	struct GameIntro {
		float time = 6.0f;
		int countDown = -1;
		bool finished = false;
	} m_intro;
	struct GameOutro {
		float time = 0.0f;
		bool finished = false;
	} m_outro;
};

/** Holds an int coordinate pair. */
struct XY { int x, y; };
/** Holds tile adjaceny information. */
struct TileAdj { bool scored[3][3] = { false, false, false, false, false, false, false, false, false }; };
/** A component representing a basic player. */
struct Score_Component : public ECSComponent<Score_Component> {
	GameBuffer * m_data = nullptr;
	int m_score = 0;
	int m_lastScore = 0;
	int m_multiplier = 0;
	int m_level = 1;
	int m_tilesCleared = 0;
	float m_timerGame = 0.0f;
	float m_timerStop = -1.0f;
	float m_timerLevelUp = 0.0f;
	float m_timerPowerOn = 0.0f;
	float m_levelLinear = 0.0f;
	float m_levelUpLinear = 0.0f;
	bool m_comboChanged = false;
	bool m_levelUp = false;
	struct ScoringData {
		std::vector<XY> xy;
		float time;
		bool scored;
	};
	std::vector<ScoringData> m_scoredTiles;
	std::vector<std::vector<TileAdj>> m_scoredAdjacency;
};

/** A component representing a basic player. */
struct Player2D_Component : public ECSComponent<Player2D_Component> {
	glm::vec3 m_rotation = glm::vec3(0.0f);
};

#endif // PUZZLE_COMPONENTS_H