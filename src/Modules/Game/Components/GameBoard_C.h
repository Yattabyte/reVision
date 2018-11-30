#pragma once
#ifndef BOARDSTATE_C_H
#define BOARDSTATE_C_H

#include "Utilities\ECS\ecsComponent.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"


/** Holds Tile State. */
struct TileState {
	// Enumerations
	enum TileType : unsigned int {
		A, B, C, D, E,
		NONE,
	} m_type = NONE;
	enum ScoreType : unsigned int {
		UNMATCHED, MATCHED, SCORED
	} m_scoreType = UNMATCHED;
	int m_tick = 0;

	TileState(const TileType & t = NONE) : m_type(t) {};
};
/** OpenGL buffer for boards. */
struct BoardBuffer {
	unsigned int types[12 * 6];
	float gravityOffsets[12 * 6];
	float lifeLinear[12 * 6];
	glm::vec3 colorScheme = glm::vec3(0.0f); float pad1;
	glm::ivec2 playerCoords = glm::ivec2(0, 0);
	float heightOffset = 0.0f;
	float gameWave = 0.0f;
	float excitementLinear = 0.0f;
	float shakeLinear = 0.0f;
	float scoreAnimLinear = 0.0f;
	float timeAnimLinear = 0.0f;
	int score = 0;
	int highlightIndex = 0;
	int multiplier = 0;
	int stopTimer = 0;
	int gameTimer = 0;
};
/** A component representing a basic player. */
struct GameBoard_Component : public ECSComponent<GameBoard_Component> {
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
	unsigned int m_rowClimbTick = 0;
	int m_playerX = 2;
	int m_playerY = 5;
	VB_Element<BoardBuffer> * m_data = nullptr;
};
/** A constructor to aid in creation. */
struct GameBoard_Constructor : ECSComponentConstructor<GameBoard_Component> {
	// (de)Constructors
	GameBoard_Constructor(VectorBuffer<BoardBuffer> * elementBuffer)
		: m_elementBuffer(elementBuffer) {};
	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new GameBoard_Component();
		component->m_data = m_elementBuffer->newElement();
		component->m_data->data->timeAnimLinear = 1.0f;
		int dataIndex = 0;
		for (int y = 0; y < 12; ++y)
			for (int x = 0; x < 6; ++x) {
				component->m_data->data->types[dataIndex] = TileState::TileType::NONE;
				component->m_data->data->gravityOffsets[dataIndex] = 0.0f;
				component->m_data->data->lifeLinear[++dataIndex] = 0.0f;
			}
		/*
			  C
			 CAB
			 AEA
			CEDDB
			EBBCDB			
			AAABCC
		*/		
		component->m_tiles[0][0].m_type = TileState::A;
		component->m_tiles[0][1].m_type = TileState::A;
		component->m_tiles[0][2].m_type = TileState::A;
		component->m_tiles[0][3].m_type = TileState::B;
		component->m_tiles[0][4].m_type = TileState::C;
		component->m_tiles[0][5].m_type = TileState::C;
		component->m_tiles[1][0].m_type = TileState::E;
		component->m_tiles[1][1].m_type = TileState::B;
		component->m_tiles[1][2].m_type = TileState::B;
		component->m_tiles[1][3].m_type = TileState::C;
		component->m_tiles[1][4].m_type = TileState::D;
		component->m_tiles[1][5].m_type = TileState::B;
		component->m_tiles[2][0].m_type = TileState::C;
		component->m_tiles[2][1].m_type = TileState::E;
		component->m_tiles[2][2].m_type = TileState::D;
		component->m_tiles[2][3].m_type = TileState::D;
		component->m_tiles[2][4].m_type = TileState::B;
		component->m_tiles[3][1].m_type = TileState::A;
		component->m_tiles[3][2].m_type = TileState::E;
		component->m_tiles[3][3].m_type = TileState::A;
		component->m_tiles[4][1].m_type = TileState::C;
		component->m_tiles[4][2].m_type = TileState::A;
		component->m_tiles[4][3].m_type = TileState::B;
		component->m_tiles[5][2].m_type = TileState::C;
		return { component, component->ID };
	}


private:
	// Private Attributes
	VectorBuffer<BoardBuffer> * m_elementBuffer = nullptr;
};

#endif // BOARDSTATE_C_H