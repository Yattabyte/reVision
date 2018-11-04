#pragma once
#ifndef BOARDSTATE_C_H
#define BOARDSTATE_C_H

#include "Utilities\ECS\ecsComponent.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"


/** Holds an int coordinate pair. */
struct XY { int x, y; };
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
	glm::mat4 tileMats[12 * 6];
	unsigned int types[12 * 6];
	float lifeTick[12 * 6];
	glm::mat4 boardMat;
	float heightOffset;
	float excitement;
	glm::vec2 padding;
	glm::mat4 playerMat;
};
/** A component representing a basic player. */
struct BoardState_Component : public ECSComponent<BoardState_Component> {
	TileState m_tiles[12][6];
	unsigned int m_rowClimbTick = 0;
	int m_playerX = 0;
	int m_playerY = 1;
	float m_excitement = 0;
	int m_score = 0;
	int m_animtedScore = 0;
	VB_Element<BoardBuffer> * m_data = nullptr;
	std::vector<std::pair<std::vector<XY>, bool>> m_scoredTiles;
};
/** A constructor to aid in creation. */
struct BoardState_Constructor : ECSComponentConstructor<BoardState_Component> {
	// (de)Constructors
	BoardState_Constructor(VectorBuffer<BoardBuffer> * elementBuffer)
		: m_elementBuffer(elementBuffer) {};
	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new BoardState_Component();
		component->m_data = m_elementBuffer->newElement();
		int dataIndex = 0;
		for (int y = 0; y < 12; ++y)
			for (int x = 0; x < 6; ++x) {
				component->m_data->data->tileMats[dataIndex] = glm::scale(glm::mat4(1.0f), glm::vec3(64.0f, 64.0f, 64.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3((x * 2) + 1, (y * 2) - 1, 0));
				component->m_data->data->types[dataIndex] = TileState::TileType::NONE;
				component->m_data->data->lifeTick[dataIndex] = 0.0f;
				dataIndex++;
			}
		component->m_data->data->types[0] = 0u;
		component->m_data->data->types[1] = 0u;
		component->m_data->data->types[2] = 0u;
		component->m_tiles[0][0].m_type = TileState::A;
		component->m_tiles[0][1].m_type = TileState::A;
		component->m_tiles[0][2].m_type = TileState::A;
		component->m_tiles[1][1].m_type = TileState::B;
		component->m_data->data->boardMat = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(3, 6, 1));
		component->m_data->data->playerMat = glm::scale(glm::mat4(1.0f), glm::vec3(64.0f, 64.0f, 64.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(1,1, 0));
		component->m_data->data->heightOffset = 0.0f;
		return { component, component->ID };
	}


private:
	// Private Attributes
	VectorBuffer<BoardBuffer> * m_elementBuffer = nullptr;
};

#endif // BOARDSTATE_C_H