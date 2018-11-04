#pragma once
#ifndef BOARD_S_H
#define BOARD_S_H 
#define _USE_MATH_DEFINES

#include "Utilities\ECS\ecsSystem.h"
#include "Modules\Game\Components\BoardState_C.h"
#include "Engine.h"
#include <random>
#include <math.h>


constexpr unsigned int BOARD_WIDTH = 6;
constexpr unsigned int BOARD_HEIGHT = 12;
constexpr int TickCount_NewLine = 500u;
constexpr int TickCount_Scoring = 50u;
constexpr int TickCount_Popping = 15u;
constexpr int ScoreFlashAmt = 4u;

/** A system that updates the rendering state for spot lighting, using the ECS system. */
class Board_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Board_System() = default;
	Board_System(Engine * engine) : m_actionState(&engine->getActionState()) {
		// Declare component types used
		addComponentType(BoardState_Component::ID);
		m_tileDistributor = std::uniform_int_distribution<unsigned int>(TileState::TileType::A, TileState::TileType::E);		
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		m_timeAccumulator += deltaTime;
		for each (const auto & componentParam in components) {
			BoardState_Component & board = *(BoardState_Component*)componentParam[0];
			
			constexpr float dt = 0.01f;
			while (m_timeAccumulator >= dt) {
				// Row climbing
				if (board.m_rowClimbTick >= TickCount_NewLine && !(board.m_scoredTiles.size())) {
					pushNewRow(board);
					board.m_rowClimbTick = 0;
				}
				else {
					// Every Tick
					if (!(board.m_scoredTiles.size()))
						board.m_rowClimbTick++;
					validateBoard(board);
					gravityBoard(board);
					scoreTiles(board);
					board.m_excitement -= 0.003f;
					if (board.m_score - board.m_animtedScore > 0) 
						board.m_animtedScore++;					
				}		

				m_timeAccumulator -= dt;
			}

			board.m_excitement = std::max(0.0f, std::min(1.1f, board.m_excitement));
			board.m_data->data->excitement = board.m_excitement;
			board.m_data->data->heightOffset = (board.m_rowClimbTick / ((float)TickCount_NewLine / 2.0f));
			
			userInteractWithBoard(board);

			// Sync board state to GPU
			for (int y = 0; y < 12; ++y)
				for (int x = 0; x < 6; ++x)
					board.m_data->data->types[(y * 6) + x] = board.m_tiles[y][x].m_type;

			// Sync player state to GPU
			board.m_playerX = std::min(4, std::max(0, board.m_playerX));
			board.m_playerY = std::min(11, std::max(1, board.m_playerY));
			board.m_data->data->playerMat = glm::scale(glm::mat4(1.0f), glm::vec3(64.0f, 64.0f, 64.0f)) * glm::translate(glm::mat4(1.0f),
				glm::vec3((board.m_playerX * 2) + 1, ((board.m_playerY) * 2) - 1, 0));
		}
	}


private:
	// Private structures
	/** Contains a unique set of coordinates coresponding to tiles that have been scored. */
	struct ScoringManifold : std::vector<XY> {
		inline void insert(const XY & newTile) {
			for each (const auto & tile in *this)
				if (tile.x == newTile.x && tile.y == newTile.y)
					return;
			push_back(newTile);
		}
	};



	// Private Methods
	/** Adds a new row of tiles to the board provided.
	@param		board		the board to add a new row of tiles to. */
	void pushNewRow(BoardState_Component & board) {
		// Move board up 1 row
		for (int x = 0; x < BOARD_WIDTH; ++x)
			for (int y = BOARD_HEIGHT - 1; y > 0; --y)
				swapTiles(board.m_tiles[y][x], board.m_tiles[y - 1][x]);
		board.m_playerY++;

		// Replace row[0] with new row	
		for (int x = 0; x < 6; ++x)
			board.m_tiles[0][x] = TileState(TileState::TileType(m_tileDistributor(m_tileGenerator)));
	}
	/** Iterates over a scoring manifold, checking if any of its tiles are involved in any horizontal matches. 
	@param		board		the board containing the tiles of interest.
	@param		series		the scoring manifold referencing the active tiles to check againts. 
	@return					true if any further matches are found, false otherwise. */
	bool checkSeries_Horizontally(BoardState_Component & board, ScoringManifold & series) {
		int horizontalCount = 1;
		for each (const auto xy in series) {
			const auto & sTile = board.m_tiles[xy.y][xy.x].m_type;
			for (int n = xy.x + 1; n < 6; ++n) {
				const auto & nTile = board.m_tiles[xy.y][n].m_type;
				if (sTile == nTile)
					horizontalCount++;
				else
					break;
			}
			if (horizontalCount >= 3) {
				const size_t startSize = series.size();
				for (int n = 1; n < horizontalCount; ++n) 
					series.insert({ xy.x + n, xy.y });				
				return series.size() != startSize;
			}
		}
		return false;
	}
	/** Iterates over a scoring manifold, checking if any of its tiles are involved in any vertical matches.
	@param		board		the board containing the tiles of interest.
	@param		series		the scoring manifold referencing the active tiles to check againts.
	@return					true if any further matches are found, false otherwise. */
	bool checkSeries_Vertically(BoardState_Component & board, ScoringManifold & series) {
		int verticalCount = 1;
		for each (const auto xy in series) {
			const auto & sTile = board.m_tiles[xy.y][xy.x].m_type;
			for (int n = xy.y + 1; n < 12; ++n) {
				const auto & nTile = board.m_tiles[n][xy.x].m_type;
				if (sTile == nTile)
					verticalCount++;
				else
					break;
			}
			if (verticalCount >= 3) {
				const size_t startSize = series.size();
				for (int n = 1; n < verticalCount; ++n)
					series.insert({ xy.x, xy.y + n });
				return series.size() != startSize;
			}
		}
		return false;
	}
	/** Begin checking a board for matching tiles of 3-of-a-kind or greater. Divided into a horizontal and vertical check, forming a scoring manifold.
	@param		board		the board containing the tiles of interest. */
	void validateBoard(BoardState_Component & board) {
 		ScoringManifold scoringManifold;
		for (int y = 1; y < 12; ++y)
			for (int x = 0; x < 6; ++x) {
				const auto & xTile = board.m_tiles[y][x];
				if (xTile.m_type == TileState::NONE || xTile.m_scoreType != TileState::UNMATCHED)
					continue;
				int countPerRow = 1;
				int countPerColumn = 1;
				for (int n = x + 1; n < 6; ++n) {
					const auto & nTile = board.m_tiles[y][n].m_type;
					if (xTile.m_type == nTile)
						countPerRow++;
					else
						break;
				}
				for (int n = y + 1; n < 12; ++n) {
					const auto & nTile = board.m_tiles[n][x].m_type;
					if (xTile.m_type == nTile)
						countPerColumn++;
					else
						break;
				}
				if (countPerRow >= 3 || countPerColumn >= 3) {
					scoringManifold.insert({ x, y });
					while (checkSeries_Horizontally(board, scoringManifold) || checkSeries_Vertically(board, scoringManifold)) {}
				}

				// Prepare tiles for scoring
				if (scoringManifold.size()) {
					board.m_scoredTiles.push_back(std::make_pair(scoringManifold, false));
					for each (const auto & xy in scoringManifold)
						board.m_tiles[xy.y][xy.x].m_scoreType = TileState::MATCHED;					
				}
				scoringManifold.clear();
			}
	}
	/** Try to delete any scored tiles if they've ticked long enough.
	@param		board		the board containing the tiles of interest. */
	void scoreTiles(BoardState_Component & board) {
		for (size_t x = 0; x < board.m_scoredTiles.size(); ++x) {
			auto & manifold = board.m_scoredTiles[x];
			// If this manifold hasn't been processed
			if (!manifold.second) {
				manifold.second = true;
				int offset = 0;
				for each (const auto & xy in manifold.first) {
					// Assign tick timing
					board.m_tiles[xy.y][xy.x].m_tick = (TickCount_Popping * offset--) - TickCount_Scoring;
					// Set as matched
					board.m_tiles[xy.y][xy.x].m_scoreType = TileState::MATCHED;
				}
				board.m_excitement += (0.075f * (float)manifold.first.size());

				// Add another 10 bonus points for every extra tile past 3, plus a base amount of 10
				if (manifold.first.size() > 3) 
					board.m_score += 10 + (10 * (manifold.first.size() - 3));				
			}
		}

		// Tick all matched tiles until they pop
		for (int y = 1; y < 12; ++y)
			for (int x = 0; x < 6; ++x) {
				auto & xTile = board.m_tiles[y][x];
				if (xTile.m_scoreType != TileState::UNMATCHED) {
					if (xTile.m_scoreType == TileState::MATCHED) {
						if (xTile.m_tick >= TickCount_Popping) {
							board.m_score += 10;
							xTile.m_scoreType = TileState::SCORED;
						}
					}
					board.m_data->data->lifeTick[(y * 6) + x] = ++xTile.m_tick;
				}
			}
		
		// Check if all tiles in a scoring set have been popped
		// If so, empty and reset them
		for (size_t x = 0; x < board.m_scoredTiles.size(); ++x) {
			auto & pair = board.m_scoredTiles[x];
			bool allPopped = true;
			for each (const auto & xy in pair.first) 
				if (board.m_tiles[xy.y][xy.x].m_scoreType != TileState::SCORED) {
					allPopped = false;
					break;
				}
			if (allPopped) {
				// Reset the tiles, make them background spaces
				for each (const auto & xy in pair.first) {
					board.m_tiles[xy.y][xy.x].m_type = TileState::NONE;
					board.m_tiles[xy.y][xy.x].m_scoreType = TileState::UNMATCHED;
					board.m_tiles[xy.y][xy.x].m_tick = 0;
					board.m_data->data->lifeTick[(xy.y * 6) + xy.x] = 0.0f;
				}
				board.m_scoredTiles.erase(board.m_scoredTiles.begin() + x);
				x--;
			}
		}
		
	}
	/** Try to drop tiles if they have room beneath them.
	@param		board		the board containing the tiles of interest. */
	void gravityBoard(BoardState_Component & board) {
		for (int y = 2; y < 12; ++y)
			for (int x = 0; x < 6; ++x) {
				const auto & xTile = board.m_tiles[y][x].m_type;
				if (xTile == TileState::NONE)
					continue;
				size_t dropIndex = y;
				while (board.m_tiles[dropIndex - 1][x].m_type == TileState::NONE) 
					dropIndex--;				
				swapTiles(board.m_tiles[y][x], board.m_tiles[dropIndex][x]);
			}
	}

	/** Swap 2 tiles if they active and not scored.
	@param		tile1		the first tile, swaps with the second.
	@param		tile2		the second tile, swaps with the first. */
	void swapTiles(TileState & tile1, TileState & tile2) {	
		if (tile1.m_scoreType != TileState::UNMATCHED || tile2.m_scoreType != TileState::UNMATCHED)
			return;
		auto copy = tile1;
		tile1 = tile2;
		tile2 = copy;
	}
	/** Apply user interaction with the board.
	@param		board		the board containing the tiles of interest. */
	void userInteractWithBoard(BoardState_Component & board) {
		// Move Left
		if (m_actionState->at(ActionState::LEFT) > 0.5f) {
			if (!m_keyPressStates[ActionState::LEFT]) {
				board.m_playerX--;
				m_keyPressStates[ActionState::LEFT] = true;
			}
		}
		else
			m_keyPressStates[ActionState::LEFT] = false;
		// Move Right
		if (m_actionState->at(ActionState::RIGHT) > 0.5f) {
			if (!m_keyPressStates[ActionState::RIGHT]) {
				board.m_playerX++;
				m_keyPressStates[ActionState::RIGHT] = true;
			}
		}
		else
			m_keyPressStates[ActionState::RIGHT] = false;
		// Move Down
		if (m_actionState->at(ActionState::BACKWARD) > 0.5f) {
			if (!m_keyPressStates[ActionState::BACKWARD]) {
				board.m_playerY--;
				m_keyPressStates[ActionState::BACKWARD] = true;
			}
		}
		else
			m_keyPressStates[ActionState::BACKWARD] = false;
		// Move Up
		if (m_actionState->at(ActionState::FORWARD) > 0.5f) {
			if (!m_keyPressStates[ActionState::FORWARD]) {
				board.m_playerY++;
				m_keyPressStates[ActionState::FORWARD] = true;
			}
		}
		else
			m_keyPressStates[ActionState::FORWARD] = false;
		// Swap Tiles
		if (m_actionState->at(ActionState::JUMP) > 0.5f) {
			if (!m_keyPressStates[ActionState::JUMP]) {
				swapTiles(board.m_tiles[board.m_playerY][board.m_playerX], board.m_tiles[board.m_playerY][board.m_playerX + 1]);
				m_keyPressStates[ActionState::JUMP] = true;
			}
		}
		else
			m_keyPressStates[ActionState::JUMP] = false;
		// Fast Forward
		if (m_actionState->at(ActionState::RUN) > 0.5f) {
			if (!m_keyPressStates[ActionState::RUN]) {
				m_keyPressStates[ActionState::RUN] = true;
				if (!board.m_scoredTiles.size()) {
					board.m_rowClimbTick = 0;
					pushNewRow(board);
				}
			}
		}
		else
			m_keyPressStates[ActionState::RUN] = false;
	}


	// Private Attributes
	float m_timeAccumulator = 0.0f;
	std::uniform_int_distribution<unsigned int> m_tileDistributor;
	std::default_random_engine m_tileGenerator;
	ActionState * m_actionState = nullptr;
	std::map<ActionState::ACTION_ENUM, bool> m_keyPressStates;
};

#endif // BOARD_S_H