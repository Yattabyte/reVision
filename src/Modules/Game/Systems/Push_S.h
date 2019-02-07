#pragma once
#ifndef PUSH_S_H
#define PUSH_S_H 

#include "Modules/Game/Systems/Interface.h"
#include "Modules/Game/Components/Board_C.h"
#include "Modules/Game/Components/Score_C.h"
#include "Modules/Game/Common_Lambdas.h"
#include <algorithm>  
#include <chrono>
#include <random>


/** Responsible for pushing new lines onto the field, climbing it higher towards the top of the board. */
class Push_System : public Game_System_Interface {
public:
	// (de)Constructors
	~Push_System() = default;
	Push_System() : m_tileGenerator((unsigned int)std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) {
		// Declare component types used
		addComponentType(Board_Component::ID);
		addComponentType(Score_Component::ID);
	}


	// Interface Implementation
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(Board_Component*)componentParam[0];
			auto & score = *(Score_Component*)componentParam[1];
			
			// Exit early if game hasn't started
			if (!board.m_gameStarted)
				continue;

			// Push new rows when timer is stopped, or ( when the user requests a new one and scored tiles have finished )
			if (!board.m_stop || (board.m_skipWaiting && (score.m_scoredTiles.size() == 0))) {
				board.m_skipWaiting = false;
				board.m_rowClimbTime += deltaTime;
				if ((board.m_rowClimbTime >= board.m_speed) && !(score.m_scoredTiles.size()))
					board.m_rowsToAdd++;
				while (board.m_rowsToAdd > 0) {
					pushNewRow(board);
					board.m_rowsToAdd--;

					// Reset height and timers
					board.m_rowClimbTime = 0.0f;
					score.m_timerStop = -1.0f;
				}
			}

			board.m_data->data->heightOffset = 2.0f * (board.m_rowClimbTime / board.m_speed);

			// Synchronize tile data to GPU
			for (int y = 0; y < 12; ++y)
				for (int x = 0; x < 6; ++x)
					board.m_data->data->tiles[(y * 6) + x].type = board.m_tiles[y][x].m_type;

			// Find lanes approaching full (top of board)
			bool critical = false;
			int largest = 0;
			for (int x = 0; x < 6; ++x) {
				int y = 11;
				for (; y >= 0; --y)
					if (board.m_tiles[y][x].m_type != TileState::NONE) {
						board.m_data->data->lanes[x] = y >= 8 ? (float(y - 8) + (board.m_data->data->heightOffset / 2.0f)) / 3.0f : 0.0f;
						if (y >= 8)
							critical = true;
						break;
					}
				if (y > largest)
					largest = y;
			}
			board.m_critical = (score.m_multiplier != 0 ? board.m_critical : critical);
			board.m_data->data->nearingTop = (float(largest) + (board.m_data->data->heightOffset / 2.0f)) / 12.0f;
		}
	}


private:
	// Private Methods
	/** Adds a new row of tiles to the board provided.
	@param		board		the board to add a new row of tiles to. */
	void pushNewRow(Board_Component & board) {
		// Move board up 1 row
		for (int x = 0; x < BOARD_WIDTH; ++x)
			for (int y = BOARD_HEIGHT - 1; y > 0; --y) {
				if (swapTiles(std::make_pair(x, y), std::make_pair(x, y - 1), board))
					board.m_tileDrops[y][x].endIndex++;
			}
		board.m_player.yPos++;

		// Replace row[0] with new row	
		for (int x = 0; x < 6; ++x) {
			std::vector<int> vector = { TileState::TileType::A, TileState::TileType::B, TileState::TileType::C, TileState::TileType::D, TileState::TileType::E};
			// Don't use the same tile as the one above this one
			if (board.m_tiles[1][x].m_type != TileState::NONE) 
				vector.erase(vector.begin() + board.m_tiles[1][x].m_type);
			// Don't use the same tile as the one before this one	
			if (x > 0 && board.m_tiles[0][x - 1].m_type != TileState::NONE) {
				auto it = std::find(vector.begin(), vector.end(), board.m_tiles[0][x - 1].m_type);
				if (it != vector.end())
					vector.erase(it);
			}
			const auto distributor = std::uniform_int_distribution<unsigned int>(0, int(vector.size())-1);
			const auto vectorIndex = distributor(m_tileGenerator);
			const auto & element = vector[vectorIndex];
			board.m_tiles[0][x] = TileState(TileState::TileType(element));
		}
	}


	// Private Attributes
	std::default_random_engine m_tileGenerator;
};

#endif // Push_System