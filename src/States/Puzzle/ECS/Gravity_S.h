#pragma once
#ifndef GRAVITY_S_H
#define GRAVITY_S_H 

#include "States/GameSystemInterface.h"
#include "States/Puzzle/ECS/components.h"
#include "States/Puzzle/Common_Lambdas.h"
#include "Assets/Sound.h"
#include "Engine.h"


/** Responsible for dropping tiles down the board. */
class Gravity_System : public Game_System_Interface {
public:
	// Public (de)Constructors
	/** Destroy this puzzle gravity system. */
	inline ~Gravity_System() = default;
	/** Construct a puzzle gravity system. */
	inline Gravity_System(Engine * engine) : m_engine(engine) {
		// Declare component types used
		addComponentType(Board_Component::ID);

		// Asset Loading
		m_soundImpact = Shared_Sound(m_engine, "Game\\impact.wav");
	}


	// Public Interface Implementation
	inline virtual bool readyToUse() override { 
		return m_soundImpact->existsYet();
	}
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(Board_Component*)componentParam[0];

			// Exit early if game hasn't started
			if (!board.m_gameInProgress)
				continue;

			// Find any tiles that should START falling
			for (unsigned int y = 2u; y < 12u; ++y)
				for (unsigned int x = 0u; x < 6u; ++x) {
					const auto & xTile = board.m_tiles[y][x];
					// Exclude any background tiles, scored tiles, or already falling tiles
					if (xTile.m_type != TileState::NONE && xTile.m_scoreType == TileState::UNMATCHED && board.m_tileDrops[y][x].dropState == Board_Component::TileDropData::STATIONARY) {

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
								board.m_tileDrops[z][x] = { Board_Component::TileDropData::FALLING, z - dropDistance, float(dropDistance), 0.0f, 0.0f, weight };
							}
						}
					}
				}

			// Cycle through all tiles
			unsigned int impactPerColumn[6] = { 0, 0, 0, 0, 0, 0 };
			for (unsigned int y = 1u; y < 12u; ++y)
				for (unsigned int x = 0u; x < 6u; ++x) {
					auto & dTile = board.m_tileDrops[y][x];
					dTile.velocity += (Tile_Gravity * dTile.weight) * deltaTime;
					dTile.time += dTile.velocity;
					switch (dTile.dropState) {
						// Find falling tiles
						case Board_Component::TileDropData::FALLING: {
							// Check if the tile has finished falling (hit something)							
							if (dTile.time >= (dTile.delta * Tile_DropDuration)) {
								// Tile has finished falling, start bouncing
								dTile.dropState = Board_Component::TileDropData::BOUNCING;
								if (impactPerColumn[x] <= 0)
									impactPerColumn[x] += dTile.weight;

								// Grab all attributes before we swap the tiles (dTile is a reference, not a copy)
								swapTiles(std::make_pair(x, y), std::make_pair(x, dTile.endIndex), board);
							}
							board.m_data->data->tiles[(y * 6) + x].yOffset = 2.0f * ((dTile.time / (dTile.delta * Tile_DropDuration)) * dTile.delta);
							break;
						}
						// Find Bouncing Tiles
						case Board_Component::TileDropData::BOUNCING: {
							// Check if the tile has finished bouncing
							const float bounceHeight = -(std::max<float>(6.0f, 12.0f - float(dTile.weight))) / 12.0f;
							const float bounceTime = (Tile_BounceDuration * (12.0f - float(dTile.weight)));
							const float adjustedTime = (dTile.time - (dTile.delta * Tile_DropDuration)) + (bounceTime * (1.0f / 2.75f));
							if (adjustedTime >= bounceTime) {
								// Tile has finished bouncing
								dTile.time = 0.0f;
								dTile.dropState = Board_Component::TileDropData::STATIONARY;
							}
							// Tile has already been dropped to destination, need to move tile upwards now, not down
							board.m_data->data->tiles[(y * 6) + x].yOffset = 2.0f * (1.0f - (easeOutBounce(adjustedTime / bounceTime))) * bounceHeight;
							break;
						}
						// Find Stationary Tiles
						default: {
							// Reset the data just in case
							board.m_data->data->tiles[(y * 6) + x].yOffset = 0.0f;
							board.m_tileDrops[y][x] = Board_Component::TileDropData();
							break;
						}
					}
				}
						
			// Play impact sounds
			for (unsigned int x = 0u; x < 6u; ++x) 
				if (impactPerColumn[x] > 0.0f){
					const float impactVolume = (impactPerColumn[x] / 12.0f); // Preportional to num of tiles falling
					const float impactSpeed = 1.0f + ((2.0f * (1.0f - impactVolume) - 1.0f) * 0.5f); // Reverse preportional to num of tiles falling, from 0.5 to 1.5
					m_engine->getManager_Sounds().playSound(m_soundImpact, impactVolume, impactSpeed);
				}
		}
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Sound m_soundImpact;
};

#endif // GRAVITY_S_H