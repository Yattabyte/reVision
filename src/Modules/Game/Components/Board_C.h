#pragma once
#ifndef BOARD_C_H
#define BOARD_C_H

#include "Utilities\ECS\ecsComponent.h"
#include "Modules\Game\Common_Definitions.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"


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
	VB_Element<GameBuffer> * m_data = nullptr;
	TileState m_tiles[12][6];
	struct TileDropData {
		enum DropState {
			STATIONARY, FALLING, BOUNCING
		} dropState = STATIONARY;
		unsigned int endIndex = 0;
		float delta = 0.0f;
		float tick = 0.0f;
		unsigned int weight = 0;
		float fallSpeed = 1.0f;
	} m_tileDrops[12][6];
	unsigned int m_gameTick = 0;
	float m_rowClimbTime = 0.0f;
	float m_speed = 1.0F;
	int m_playerX = 0;
	int m_playerY = 0;
	int m_rowsToAdd = 0;
	bool m_critical = false;
	bool m_stop = false;
	bool m_skipWaiting = false;
	bool m_gameStarted = false;
	struct GameMusic {
		float accumulator = 0.0f;
		float beatSeconds = 0.0f;
		bool beat = false;
	} m_music;
	struct GameIntro {
		float time = 6.0f;
		int countDown = -1;
		bool start = false;
		bool finished = false;
	} m_intro;
};
/** A constructor to aid in creation. */
struct Board_Constructor : ECSComponentConstructor<Board_Component> {
	// (de)Constructors
	Board_Constructor(VB_Element<GameBuffer> * gameData)
		: m_gameData(gameData) {};
	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new Board_Component();
		component->m_data = m_gameData;
		return { component, component->ID };
	}
private:
	// Private Attributes
	VB_Element<GameBuffer> * m_gameData = nullptr;
};

#endif // BOARD_C_H