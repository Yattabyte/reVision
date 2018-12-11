#pragma once
#ifndef PUSH_S_H
#define PUSH_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Modules\Game\Common.h"
#include "Modules\Game\Components\GameBoard_C.h"
#include "Modules\Game\Components\GameScore_C.h"
#include <random>


/** Responsible for pushing new lines onto the field, climbing it higher towards the top of the board. */
class Push_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Push_System() = default;
	Push_System() {
		// Declare component types used
		addComponentType(GameBoard_Component::ID);
		addComponentType(GameScore_Component::ID);
		m_tileDistributor = std::uniform_int_distribution<unsigned int>(TileState::TileType::A, TileState::TileType::E);
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(GameBoard_Component*)componentParam[0];
			auto & score = *(GameScore_Component*)componentParam[1];

			// Tick row-climbing
			if (!board.m_stop) {
				board.m_rowClimbTick += board.m_speed;
				if (board.m_rowClimbTick >= double(TickCount_NewLine) && !(score.m_scoredTiles.size()))
					board.m_rowsToAdd++;
				while (board.m_rowsToAdd > 0) {
					pushNewRow(board);
					board.m_rowsToAdd--;

					// Reset height and ticks
					board.m_rowClimbTick = 0.0;
					score.m_stopTimeTick = 0;
					score.m_stopTimer = -1;
				}
			}

			board.m_data->data->heightOffset = 2.0f * (float(board.m_rowClimbTick) / (float)TickCount_NewLine);

			// Synchronize tile data to GPU
			for (int y = 0; y < 12; ++y)
				for (int x = 0; x < 6; ++x)
					board.m_data->data->types[(y * 6) + x] = board.m_tiles[y][x].m_type;

			// Find lanes approaching full (top of board)
			board.m_nearingTop = false;
			for (int x = 0; x < 6; ++x)
				for (int y = 11; y >= 0; --y)
					if (board.m_tiles[y][x].m_type != TileState::NONE) {
						board.m_data->data->lanes[x] = y >= 8 ? (float(y - 8) + (board.m_data->data->heightOffset / 2.0f)) / 3.0f : 0.0f;
						if (y >= 8)
							board.m_nearingTop = true;
						break;
					}
		}
	}


private:
	// Private Methods
	/** Adds a new row of tiles to the board provided.
	@param		board		the board to add a new row of tiles to. */
	void pushNewRow(GameBoard_Component & board) {
		// Move board up 1 row
		for (int x = 0; x < BOARD_WIDTH; ++x)
			for (int y = BOARD_HEIGHT - 1; y > 0; --y)
				swapTiles(std::make_pair(x, y), std::make_pair(x, y - 1), board);
		board.m_playerY++;

		// Replace row[0] with new row	
		for (int x = 0; x < 6; ++x)
			board.m_tiles[0][x] = TileState(TileState::TileType(m_tileDistributor(m_tileGenerator)));
	}


	// Private Attributes
	std::uniform_int_distribution<unsigned int> m_tileDistributor;
	std::default_random_engine m_tileGenerator;
};

#endif // Push_System