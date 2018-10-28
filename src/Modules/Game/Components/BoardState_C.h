#pragma once
#ifndef BOARDSTATE_C_H
#define BOARDSTATE_C_H

#include "Utilities\ECS\ecsComponent.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"


struct TileState {
	enum TileType : unsigned int {
		A, B, C, D, E,
		NONE,
	} m_type = NONE;
	TileState(const TileType & t = NONE) : m_type(t) {}
};
/** OpenGL buffer for boards. */
struct BoardBuffer {
	glm::mat4 tileMats[12 * 6];
	unsigned int types[12 * 6];
	glm::mat4 boardMat;
	unsigned int tick;
	glm::vec3 padding;
	glm::mat4 playerMat;
};
/** A component representing a basic player. */
struct BoardState_Component : public ECSComponent<BoardState_Component> {
	TileState m_tiles[12][6];
	unsigned int m_ticks = 0;
	int m_playerX = 0;
	int m_playerY = 1;
	VB_Element<BoardBuffer> * m_data = nullptr;
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
				component->m_data->data->types[dataIndex++] = TileState::TileType::NONE;
			}
		component->m_data->data->types[0] = 0u;
		component->m_data->data->types[7] = 1u;
		component->m_tiles[0][0].m_type = TileState::A;
		component->m_tiles[1][1].m_type = TileState::B;
		component->m_data->data->boardMat = glm::scale(glm::mat4(1.0f), glm::vec3(3, 6, 1));
		component->m_data->data->tick = 0;
		component->m_data->data->playerMat = glm::scale(glm::mat4(1.0f), glm::vec3(64.0f, 64.0f, 64.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(1,1, 0));
		return { component, component->ID };
	}


private:
	// Private Attributes
	VectorBuffer<BoardBuffer> * m_elementBuffer = nullptr;
};

#endif // BOARDSTATE_C_H