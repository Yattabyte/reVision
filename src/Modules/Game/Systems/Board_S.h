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
constexpr int TickCount_GameAnimation = 750u;
constexpr int TickCount_NewLine = 500u;
constexpr float TickCount_TileDrop = 10.0F;
constexpr float TickCount_TileBounce = 15.0F;

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

			// Allow user to interact with the board
			userInteractWithBoard(board, score);
			
			// Tick row-climbing
			if (!(score.m_scoredTiles.size()) && score.m_stopTimer <= 0) {
				board.m_rowClimbTick++;
				if (board.m_rowClimbTick >= TickCount_NewLine && !(score.m_scoredTiles.size())) 
					pushNewRow(board, score);				
			}

			// Tick stop-timer
			if (++score.m_stopTimeTick >= 100) {
				if (score.m_stopTimer > 0)
					score.m_stopTimer--;
				score.m_stopTimeTick = 0;
			}

			// Tick gravity
			gravityBoard(board);

			// Synchronize component data to GPU
			board.m_data->data->gameTick = ++board.m_data->data->gameTick >= TickCount_GameAnimation ? 0 : board.m_data->data->gameTick;
			board.m_data->data->excitement = std::max(0.0f, std::min(1.1f, board.m_data->data->excitement -= 0.001f));
			board.m_data->data->heightOffset = 2.0f * (board.m_rowClimbTick / (float)TickCount_NewLine);
			for (int y = 0; y < 12; ++y)
				for (int x = 0; x < 6; ++x)
					board.m_data->data->types[(y * 6) + x] = board.m_tiles[y][x].m_type;
			board.m_playerX = std::min(4, std::max(0, board.m_playerX));
			board.m_playerY = std::min(11, std::max(1, board.m_playerY));
			board.m_data->data->playerCoords = glm::ivec2(board.m_playerX, board.m_playerY);
		}
	}


private:
	// Private Methods
	/** Swap 2 tiles ONLY if they're active, not falling, and not scored
	@param		tile1		the first tile, swaps with the second.
	@param		tile2		the second tile, swaps with the first. */
	static constexpr auto swapTiles = [](const auto & coordsA, const auto & coordsB, GameBoard_Component & board) {
		auto & tileState1 = board.m_tiles[coordsA.second][coordsA.first];
		auto & tileState2 = board.m_tiles[coordsB.second][coordsB.first];
		auto & tileDrop1 = board.m_tileDrops[coordsA.second][coordsA.first];
		auto & tileDrop2 = board.m_tileDrops[coordsB.second][coordsB.first];
		if (tileState1.m_scoreType != TileState::UNMATCHED || tileState2.m_scoreType != TileState::UNMATCHED ||
			tileDrop1.dropState == GameBoard_Component::TileDropData::FALLING || tileDrop2.dropState == GameBoard_Component::TileDropData::FALLING)
			return;

		// Swap mechanism
		auto copyState = tileState1;
		tileState1 = tileState2;
		tileState2 = copyState;
		auto copyDrop = tileDrop1;
		tileDrop1 = tileDrop2;
		tileDrop2 = copyDrop;
	};
	/** Adds a new row of tiles to the board provided.
	@param		board		the board to add a new row of tiles to. */
	void pushNewRow(GameBoard_Component & board, GameScore_Component & score) {
		if (!score.m_scoredTiles.size()) {
			// Move board up 1 row
			for (int x = 0; x < BOARD_WIDTH; ++x)
				for (int y = BOARD_HEIGHT - 1; y > 0; --y)
					swapTiles(std::make_pair(x, y), std::make_pair(x, y - 1), board);
			board.m_playerY++;

			// Replace row[0] with new row	
			for (int x = 0; x < 6; ++x)
				board.m_tiles[0][x] = TileState(TileState::TileType(m_tileDistributor(m_tileGenerator)));

			board.m_rowClimbTick = 0;
			score.m_stopTimeTick = 0;
			score.m_stopTimer = 0;
		}
	}	
	/** Try to drop tiles if they have room beneath them.
	@param		board		the board containing the tiles of interest. */
	void gravityBoard(GameBoard_Component & board) {	
		// Bouncing Easing Function
		static constexpr auto easeOutBounce = [](auto t) {
			if (t < (1 / 2.75))
				return (7.5625 * t *t);
			else if (t < (2 / 2.75))
				return (7.5625 * (t -= (1.5 / 2.75)) * t + .75);
			else if (t < (2.5 / 2.75))
				return (7.5625 * (t -= (2.25 / 2.75)) * t + .9375);
			else
				return (7.5625 * (t -= (2.625 / 2.75)) * t + .984375);
		};

		// Find any tiles that should START falling
		for (unsigned int y = 2u; y < 12u; ++y)
			for (unsigned int x = 0u; x < 6u; ++x) {
				const auto & xTile = board.m_tiles[y][x];
				// Exclude any background tiles, scored tiles, or already falling tiles
				if (xTile.m_type != TileState::NONE && xTile.m_scoreType == TileState::UNMATCHED && board.m_tileDrops[y][x].dropState == GameBoard_Component::TileDropData::STATIONARY) {

					// Determine how far this tile may fall
					unsigned int dropIndex = y;
					while (dropIndex - 1u > 0 && board.m_tiles[dropIndex - 1u][x].m_type == TileState::NONE)
						dropIndex--;
					unsigned int dropDistance = y - dropIndex;
					const auto & dTile = board.m_tiles[dropIndex][x];

					// Determine the amount of weight imposed on this tile
					unsigned int weight = 0;
					for (unsigned int z = y; z < 12u; ++z) {
						if (board.m_tiles[z][x].m_scoreType == TileState::SCORED)
							break;
						if (board.m_tiles[z][x].m_type == TileState::NONE)
							continue;
						weight++;
					}

					// Continue only if this tile WILL ACTUALLY FALL
					if ((dropDistance > 0u) && (dTile.m_scoreType == TileState::UNMATCHED)) {
						// Mark this tile and all directly above this as falling
						for (unsigned int z = y; z < 12u; ++z) {
							// Add a skip for dead space
							if (board.m_tiles[z][x].m_type == TileState::NONE) {
								dropDistance++;
								continue;
							}
							// Don't move scored tiles
							if (board.m_tiles[z][x].m_scoreType == TileState::SCORED)
								break;
							board.m_tileDrops[z][x] = { GameBoard_Component::TileDropData::FALLING, z - dropDistance, float(dropDistance), 0.0f, weight };
						}
					}
				}
			}

		// Cycle through all tiles
		for (unsigned int y = 1u; y < 12u; ++y)
			for (unsigned int x = 0u; x < 6u; ++x) {
				auto & dTile = board.m_tileDrops[y][x];
				switch (dTile.dropState) {
					// Find falling tiles
					case GameBoard_Component::TileDropData::FALLING: {
						// Increment the tile tick and check if it has finished falling (hit something)
						if (dTile.tick >= (dTile.delta * TickCount_TileDrop)) {
							// Tile has finished falling, start bouncing
							dTile.dropState = GameBoard_Component::TileDropData::BOUNCING;
							swapTiles(std::make_pair(x, y), std::make_pair(x, dTile.endIndex), board);
						}
						board.m_data->data->gravityOffsets[(y * 6) + x] = 2.0f * ((dTile.tick / (dTile.delta * TickCount_TileDrop)) * dTile.delta);
						dTile.tick += (dTile.fallSpeed += 0.1f);
						break;
					}
					// Find Bouncing Tiles
					case GameBoard_Component::TileDropData::BOUNCING: {
						const float bounceTime = (TickCount_TileBounce * (12.0f - float(dTile.weight))) / dTile.fallSpeed;
						const float adjustedTick = (dTile.tick - (dTile.delta * TickCount_TileDrop)) + (bounceTime * (1.0f / 2.75f));
						// Increment the tile tick and check if it has finished bouncing
						if (adjustedTick >= bounceTime) {
							// Tile has finished bouncing
							dTile.tick = 0;
							dTile.dropState = GameBoard_Component::TileDropData::STATIONARY;
						}
						// Tile has already been dropped to destination, need to move tile upwards now, not down
						// So use negative reciprical of bounce function to get desired effect
						board.m_data->data->gravityOffsets[(y * 6) + x] = -((std::max(6.0f, 12.0f - float(dTile.weight))) / 6.0f) * (1.0f - (easeOutBounce(adjustedTick / bounceTime)));
						dTile.tick++;
						break;
					}
					// Find Stationary Tiles
					default: {
						// Reset the data just in case
						board.m_data->data->gravityOffsets[(y * 6) + x] = 0;
						board.m_tileDrops[y][x] = GameBoard_Component::TileDropData();
						break;
					}
				}
			}
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
				swapTiles(std::make_pair(board.m_playerX, board.m_playerY), std::make_pair(board.m_playerX + 1, board.m_playerY), board);
				m_keyPressStates[ActionState::JUMP] = true;
			}
		}
		else
			m_keyPressStates[ActionState::JUMP] = false;
		// Fast Forward
		if (m_actionState->at(ActionState::RUN) > 0.5f) {
			if (!m_keyPressStates[ActionState::RUN]) {
				m_keyPressStates[ActionState::RUN] = true;				
				pushNewRow(board, score);				
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