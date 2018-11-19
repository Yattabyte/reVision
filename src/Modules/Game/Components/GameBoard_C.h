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
	float lifeTick[12 * 6];
	glm::ivec2 playerCoords = glm::ivec2(0, 0);
	float heightOffset = 0.0f;
	float excitement = 0.0f;
	int score = 0;
	int scoreTick = 0;
	int highlightIndex = 0;
	int stopTimer = 0;
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
		int dataIndex = 0;
		for (int y = 0; y < 12; ++y)
			for (int x = 0; x < 6; ++x) {
				component->m_data->data->types[dataIndex] = TileState::TileType::NONE;
				component->m_data->data->gravityOffsets[dataIndex] = 0.0f;
				component->m_data->data->lifeTick[dataIndex] = 0.0f;
			}
		/*
			  E
			 EDD
			EBBCD			
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
		component->m_tiles[2][1].m_type = TileState::E;
		component->m_tiles[2][2].m_type = TileState::D;
		component->m_tiles[2][3].m_type = TileState::D;
		component->m_tiles[3][2].m_type = TileState::E;
		

		/*
		component->m_tiles[0][0].m_type = TileState::A;
		component->m_tiles[1][0].m_type = TileState::B;
		component->m_tiles[2][0].m_type = TileState::A;
		component->m_tiles[3][0].m_type = TileState::B;
		component->m_tiles[4][0].m_type = TileState::A;
		component->m_tiles[5][0].m_type = TileState::B;
		component->m_tiles[6][0].m_type = TileState::A;
		component->m_tiles[7][0].m_type = TileState::B;
		component->m_tiles[8][0].m_type = TileState::A;
		component->m_tiles[9][0].m_type = TileState::B;
		component->m_tiles[10][0].m_type = TileState::A;
		component->m_tiles[11][0].m_type = TileState::B;
		*/
		return { component, component->ID };
	}


private:
	// Private Attributes
	VectorBuffer<BoardBuffer> * m_elementBuffer = nullptr;
};

#endif // BOARDSTATE_C_H