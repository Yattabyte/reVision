#pragma once
#ifndef BOARD_S_H
#define BOARD_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Modules\Game\Components\BoardState_C.h"
#include <random>


constexpr unsigned int BOARD_WIDTH = 6;
constexpr unsigned int BOARD_HEIGHT = 12;

/** A system that updates the rendering state for spot lighting, using the ECS system. */
class Board_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Board_System() = default;
	Board_System() {
		// Declare component types used
		addComponentType(BoardState_Component::ID);
		m_tileDistributor = std::uniform_int_distribution<unsigned int>(TileState::TileType::A, TileState::TileType::E);		
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		m_timeAccumulator += deltaTime;
		for each (const auto & componentParam in components) {
			BoardState_Component & boardComponent = *(BoardState_Component*)componentParam[0];
			if (m_timeAccumulator >= 0.5f) {
				if (boardComponent.m_ticks == 15) {
					pushNewRow(boardComponent);
					boardComponent.m_ticks = 0;
				}
				else 
					boardComponent.m_ticks++;
				m_timeAccumulator = 0.0f;
			}
			int dataIndex = 0;
			for (int y = 0; y < 12; ++y)
				for (int x = 0; x < 6; ++x)
					boardComponent.m_data->data->types[dataIndex++] = boardComponent.m_tiles[y][x].m_type;
			boardComponent.m_data->data->tick = boardComponent.m_ticks;
		}
	}


private:
	// Private Methods
	/** Adds a new row of tiles to the board provided.
	@param		board		the board to add a new row of tiles to. */
	void pushNewRow(BoardState_Component & board) {
		// Move board up 1 row
		for (int x = 0; x < BOARD_WIDTH; ++x) 
			for (int y = BOARD_HEIGHT - 1; y > 0; --y) 
				board.m_tiles[y][x] = board.m_tiles[y-1][x];

		// Replace row[0] with new row	
		for (int x = 0; x < 6; ++x)
			board.m_tiles[0][x] = TileState(TileState::TileType(m_tileDistributor(m_tileGenerator)));
	}


	// Private Attributes
	float m_timeAccumulator = 0.0f;
	std::uniform_int_distribution<unsigned int> m_tileDistributor;
	std::default_random_engine m_tileGenerator;
};

#endif // BOARD_S_H