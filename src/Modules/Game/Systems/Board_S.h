#pragma once
#ifndef BOARD_S_H
#define BOARD_S_H 
#define _USE_MATH_DEFINES

#include "Utilities\ECS\ecsSystem.h"
#include "Modules\Game\Components\GameBoard_C.h"
#include "Modules\Game\Components\GameScore_C.h"
#include "Engine.h"
#include <random>
#include <math.h>


constexpr unsigned int BOARD_WIDTH = 6;
constexpr unsigned int BOARD_HEIGHT = 12;
constexpr int TickCount_NewLine = 500u;

/** A system that updates the rendering state for spot lighting, using the ECS system. */
class Board_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Board_System() = default;
	Board_System(Engine * engine) : m_actionState(&engine->getActionState()) {
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
			
			// Row climbing
			if (!(score.m_scoredTiles.size()) && score.m_stopTimer <= 0)
				board.m_rowClimbTick++;
			if (board.m_rowClimbTick >= TickCount_NewLine && !(score.m_scoredTiles.size())) {
				pushNewRow(board);
				board.m_rowClimbTick = 0;
			}

			// Timer preventing row climbing
			if (score.m_stopTimeTick >= 100) {
				score.m_stopTimeTick = 0;
				score.m_stopTimer--;
			}
			if (score.m_stopTimer > 0)
				score.m_stopTimeTick++;
			board.m_data->data->stopTimer = score.m_stopTimer;
			userInteractWithBoard(board, score);
			gravityBoard(board);
			board.m_data->data->excitement = std::max(0.0f, std::min(1.1f, board.m_data->data->excitement -= 0.001f));
			board.m_data->data->heightOffset = (board.m_rowClimbTick / ((float)TickCount_NewLine / 2.0f));
			

			// Sync board state to GPU
			for (int y = 0; y < 12; ++y)
				for (int x = 0; x < 6; ++x)
					board.m_data->data->types[(y * 6) + x] = board.m_tiles[y][x].m_type;

			// Sync player state to GPU
			board.m_playerX = std::min(4, std::max(0, board.m_playerX));
			board.m_playerY = std::min(11, std::max(1, board.m_playerY));
			board.m_data->data->playerCoords = glm::ivec2(board.m_playerX, board.m_playerY);
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
				swapTiles(board.m_tiles[y][x], board.m_tiles[y - 1][x]);
		board.m_playerY++;

		// Replace row[0] with new row	
		for (int x = 0; x < 6; ++x)
			board.m_tiles[0][x] = TileState(TileState::TileType(m_tileDistributor(m_tileGenerator)));
	}
	/** Try to drop tiles if they have room beneath them.
	@param		board		the board containing the tiles of interest. */
	void gravityBoard(GameBoard_Component & board) {
		bool didAnything = false;
		for (int y = 2; y < 12; ++y)
			for (int x = 0; x < 6; ++x) {
				const auto & xTile = board.m_tiles[y][x];
				if (xTile.m_type == TileState::NONE)
					continue;
				size_t dropIndex = y;
				while (board.m_tiles[dropIndex - 1][x].m_type == TileState::NONE) 
					dropIndex--;	

				const auto & dTile = board.m_tiles[dropIndex][x];
				if (dropIndex != y && xTile.m_scoreType == TileState::UNMATCHED && dTile.m_scoreType == TileState::UNMATCHED) {
					swapTiles(board.m_tiles[y][x], board.m_tiles[dropIndex][x]);
					didAnything = true;
				}
			}

		board.m_stable = !didAnything;
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
	void userInteractWithBoard(GameBoard_Component & board, GameScore_Component & score) {
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
				if (!score.m_scoredTiles.size()) {
					board.m_rowClimbTick = 0;
					score.m_stopTimeTick = 0;
					score.m_stopTimer = 0;
					pushNewRow(board);
				}
			}
		}
		else
			m_keyPressStates[ActionState::RUN] = false;
	}


	// Private Attributes
	std::uniform_int_distribution<unsigned int> m_tileDistributor;
	std::default_random_engine m_tileGenerator;
	ActionState * m_actionState = nullptr;
	std::map<ActionState::ACTION_ENUM, bool> m_keyPressStates;
};

#endif // BOARD_S_H