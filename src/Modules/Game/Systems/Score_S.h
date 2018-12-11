#pragma once
#ifndef SCOREBOARD_S_H
#define SCOREBOARD_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Assets\Asset_Sound.h"
#include "Modules\Game\Common.h"
#include "Engine.h"

/** Component Types Used */
#include "Modules\Game\Components\GameBoard_C.h"
#include "Modules\Game\Components\GameScore_C.h"


/** Responsible for validating the game state, checking for scoreable events. */
class Score_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Score_System() = default;
	Score_System(Engine * engine) : m_engine(engine) {
		// Declare component types used
		addComponentType(GameBoard_Component::ID);
		addComponentType(GameScore_Component::ID);		

		// Asset Loading
		m_soundPop = Asset_Sound::Create(m_engine, "Game\\pop.wav");
		m_soundMultiplier = Asset_Sound::Create(m_engine, "Game\\multiplier.wav");
		m_soundMultiplierLost = Asset_Sound::Create(m_engine, "Game\\multiplier lost.wav");
		m_soundLevelGained = Asset_Sound::Create(m_engine, "Game\\level gain.wav");
	}

	
	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(GameBoard_Component*)componentParam[0];
			auto & score = *(GameScore_Component*)componentParam[1];

			bool allTilesGrounded = true;
			for (unsigned int y = 2u; y < 12u; ++y)
				for (unsigned int x = 0u; x < 6u; ++x)
					if (board.m_tileDrops[y][x].dropState == GameBoard_Component::TileDropData::FALLING) {
						allTilesGrounded = false;
						break;
					}
			if (allTilesGrounded) {
				validateBoard(board, score);
				scoreTiles(board, score);
			}

			board.m_stop = bool(score.m_scoredTiles.size() || score.m_stopTimer >= 0);

			// Animate score climbing
			if ((score.m_score - board.m_data->data->score) > 0) 
				board.m_data->data->score = std::min(score.m_score, board.m_data->data->score + 1);			
			else
				score.m_lastScore = score.m_score;
			// Highlight digits that are changing
			constexpr int decimalPlaces[8] = { 10000000,1000000,100000,10000,1000,100,10,1 };
			GLuint scoreLength = 1;
			for (GLuint x = 0; x < 8; ++x)
				if (score.m_score >= decimalPlaces[x]) {
					scoreLength = 8 - x;
					break;
				}
			const int scoreGained = (score.m_score - score.m_lastScore);
			int firstMostDigit = 8;
			for (int x = 0; x < 8; ++x)
				if (scoreGained >= decimalPlaces[x]) {
					firstMostDigit = x;
					break;
				}
			// Animate multiplier climbing
			board.m_data->data->scoreAnimLinear = score.m_multiplier > 1 ? std::max(0.0f, std::min(1.0f, board.m_data->data->scoreAnimLinear)) : 0.0f;
			
			// Synchronize component data to GPU
			board.m_data->data->shakeLinear = std::max(0.0f, std::min(1.0f, board.m_data->data->shakeLinear - 0.01f));
			board.m_data->data->highlightIndex = scoreLength - (8 - firstMostDigit);
			board.m_data->data->multiplier = score.m_multiplier;
			score.m_stopTimer = std::min(9, score.m_stopTimer);
			board.m_data->data->stopTimer = score.m_stopTimer;

			score.m_levelLinear = float(score.m_tilesCleared) / float(score.m_level * 12);
			score.m_levelUpLinear = glm::clamp(float(score.m_levelUpTick) / float(TickCount_LevelUp), 0.0f, 1.0f);
			if (score.m_levelUpTick >= 0) 
				score.m_levelUpTick = std::min(TickCount_LevelUp, score.m_levelUpTick + 1);			
			if (score.m_levelUpTick >= TickCount_LevelUp)
				score.m_levelUpTick = -1;
			if (score.m_tilesCleared >= (score.m_level * 12)) {
				score.m_tilesCleared = 0;
				score.m_levelUpTick = 0;
				score.m_level++;

				if (m_soundLevelGained->existsYet())
					m_engine->getSoundManager().playWav(m_soundLevelGained->m_soundObj, 0.75f);
			}
			board.m_speed = 1.0 + (double(score.m_level - 1) * 0.2f);
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
		static bool sortFunc(const XY & a, const XY & b) {
			if (a.y > b.y)
				return true;
			else if (a.y == b.y)
				return (a.x < b.x);
			else // (a.y < b.y)
				return false;
		}
		inline void sort() {
			std::sort(begin(), end(), sortFunc);
		}
	};


	// Private Methods
	/** Determines which scored tiles are next to each other.
	@param		tileSet	the set of tiles to act on.
	@return		a vector of tile adjacency information. */
	std::vector<TileAdj> getAdjacency(const std::vector<XY> & tileSet) {
		std::vector<TileAdj> adj(tileSet.size());
		for (size_t n = 0; n < tileSet.size(); ++n) {
			const auto & x = tileSet[n].x;
			const auto & y = tileSet[n].y;


			for (size_t m = 0; m < tileSet.size(); ++m) {
				const auto & mx = tileSet[m].x;
				const auto & my = tileSet[m].y;
				if (mx == x && my == y)
					continue;

				/*				
					[ ][ ][ ]
					[ ][+][ ]
					[+][+][+]
				*/
				if (mx == x - 1 && my == y - 1)
					adj[n].scored[0][0] = true;
				if (mx == x		&&	my == y - 1)
					adj[n].scored[0][1] = true;
				if (mx == x + 1	&&	my == y - 1)
					adj[n].scored[0][2] = true;

				/*
					[ ][ ][ ]
					[+][+][+]
					[ ][ ][ ]
				*/
				if (mx == x - 1	&&	my == y)
					adj[n].scored[1][0] = true;
				if (mx == x		&&	my == y)
					adj[n].scored[1][1] = true;
				if (mx == x + 1	&&	my == y)
					adj[n].scored[1][2] = true;

				/*
					[+][+][+]
					[ ][+][ ]
					[ ][ ][ ]
				*/
				if (mx == x - 1	&&	my == y + 1)
					adj[n].scored[2][0] = true;
				if (mx == x		&&	my == y + 1)
					adj[n].scored[2][1] = true;
				if (mx == x + 1	&&	my == y + 1)
					adj[n].scored[2][2] = true;
			}
		}
		return adj;
	}
	/** Iterates over a scoring manifold, checking if any of its tiles are involved in any horizontal matches.
	@param		board		the board containing the tiles of interest.
	@param		series		the scoring manifold referencing the active tiles to check againts.
	@return					true if any further matches are found, false otherwise. */
	bool checkSeries_Horizontally(const GameBoard_Component & board, ScoringManifold & series) {
		for each (const auto xy in series) {
			int horizontalCount = 1;
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
	bool checkSeries_Vertically(const GameBoard_Component & board, ScoringManifold & series) {
		for each (const auto xy in series) {
			int verticalCount = 1;
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
	@param		board		the board containing the tiles of interest.
	@param		score		the scoring component. */
	void validateBoard(GameBoard_Component & board, GameScore_Component & score) {
		std::vector<ScoringManifold> allMatchingSets;
		static constexpr auto findScoredTiles = [](const auto & xCoord, const auto & yCoord, GameScore_Component & score) {
			for each (const auto & pair in score.m_scoredTiles)
				for each (const auto & xy in pair.first) 
					if (xy.x == xCoord && xy.y == yCoord)
						return true;
			return false;
		};
		for (int y = 1; y < 12; ++y)
			for (int x = 0; x < 6; ++x) {
				ScoringManifold matchingSet;
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
					matchingSet.insert({ x, y });
					while (checkSeries_Horizontally(board, matchingSet) || checkSeries_Vertically(board, matchingSet)) {}
				}

				// Prepare tiles for scoring
				if (matchingSet.size()) {
					bool keepSet = true;
					for each (const auto & xy in matchingSet) 
						if (findScoredTiles(xy.x, xy.y, score)) {
							keepSet = false;
							break;
						}
					if (keepSet) {
						allMatchingSets.push_back(matchingSet);
						// Set the tiles as matched so that they aren't matched against more than once
						for each (const auto & xy in matchingSet)
							board.m_tiles[xy.y][xy.x].m_scoreType = TileState::MATCHED;
					}
				}
			}

		if (allMatchingSets.size()) {
			// All matches formed in a single tick count as a single mega set, sized as the sum of all their tiles
			// Combine their sets into 1 manifold
			ScoringManifold combinedManifold;
			for each (const auto & matchingSet in allMatchingSets)
				for each (const auto & xy in matchingSet)
					combinedManifold.push_back(xy);
			combinedManifold.sort();
			score.m_scoredTiles.push_back(std::make_pair(combinedManifold, false));	
			score.m_scoredAdjacency.push_back(getAdjacency(combinedManifold));
			score.m_comboChanged = true;
		}
	}
	/** Try to delete any scored tiles if they've ticked long enough.
	@brief		Rules: 
				- Every tile matched		+pts(10) each
					(Ex: 3 tiles += 30 pts)
				- Every tile after 3rd in set +pts(10) each, +pts(setSize)
					(Ex: 5 tiles += 25 pts)
				- For every chained combo	-> +pts(5), +multiplier(1)
				- All points added are multiplied by the combo multiplier
	@param		board		the board containing the tiles of interest.
	@param		score		the score component. */
	void scoreTiles(GameBoard_Component & board, GameScore_Component & score) {
		// Check to see if we should increment combo multiplier and add points
		if (score.m_scoredTiles.size()) {
			if (score.m_comboChanged) {
				score.m_comboChanged = false;
				addScore(score, 5);

				score.m_multiplier++;
				score.m_stopTimer++;
				board.m_data->data->shakeLinear += (score.m_multiplier / 5.0f);
				board.m_data->data->scoreAnimLinear++;

				if (m_soundMultiplier->existsYet() && score.m_multiplier >= 2)
					m_engine->getSoundManager().playWav(m_soundMultiplier->m_soundObj, 0.75f, 1.0f + (score.m_multiplier / 10.0f));
			}
		}
		else if ((!score.m_scoredTiles.size() || !score.m_comboChanged) && score.m_multiplier) {
			// Reset multiplier if no tiles are scored
			if (m_soundMultiplierLost->existsYet() && score.m_multiplier >= 2)
				m_engine->getSoundManager().playWav(m_soundMultiplierLost->m_soundObj, 0.25f);
			score.m_multiplier = 0;
			score.m_comboChanged = false;

		}

		for (size_t x = 0; x < score.m_scoredTiles.size(); ++x) {
			auto & manifold = score.m_scoredTiles[x];
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
				board.m_data->data->excitementLinear += (0.075f * (float)manifold.first.size());
				// Add time, but never move timer past 3 seconds
				score.m_stopTimer = std::min(score.m_stopTimer + 1, 3);
				// Add another 10 bonus points for every extra tile past 3, plus a base amount of 10, also add time
				if (manifold.first.size() > 3) {
					addScore(score, manifold.first.size() + (10 * (manifold.first.size() - 3)));
					board.m_data->data->shakeLinear += std::max(0.25f, manifold.first.size() / 9.0f);
					score.m_stopTimer++;
				}

				score.m_stopTimeTick = 0;
			}
		}


		// Tick all matched tiles until they pop
		for (int y = 1; y < 12; ++y)
			for (int x = 0; x < 6; ++x) {
				auto & xTile = board.m_tiles[y][x];
				if (xTile.m_scoreType == TileState::MATCHED) {
					if (xTile.m_tick >= TickCount_Popping) {
						// Tile SCORED
						xTile.m_scoreType = TileState::SCORED;
						addScore(score, 10);
						score.m_tilesCleared++;
					}
					board.m_data->data->lifeLinear[(y * 6) + x] = float(++xTile.m_tick) / float(TickCount_Popping);
				}
			}
		
		// Check if all tiles in a scoring set have been popped
		// If so, empty and reset them
		for (size_t x = 0; x < score.m_scoredTiles.size(); ++x) {
			auto & pair = score.m_scoredTiles[x];
			bool allPopped = true;
			for each (const auto & xy in pair.first)
				if (board.m_tiles[xy.y][xy.x].m_scoreType != TileState::SCORED) {
					allPopped = false;
					break;
				}
			if (allPopped) {
				// Reset the tiles and make them background spaces
				for each (const auto & xy in pair.first) {
					board.m_tiles[xy.y][xy.x].m_type = TileState::NONE;
					board.m_tiles[xy.y][xy.x].m_scoreType = TileState::UNMATCHED;
					board.m_tiles[xy.y][xy.x].m_tick = 0;
					board.m_data->data->lifeLinear[(xy.y * 6) + xy.x] = 0.0f;
				}
				score.m_scoredTiles.erase(score.m_scoredTiles.begin() + x);
				score.m_scoredAdjacency.erase(score.m_scoredAdjacency.begin() + x);
				x--;
			}
		}

	}
	/** A wrapper function for adding points to the game score. Accounts for any other variables like multipliers.
	@param	score	the score component.
	@param	amount	the amount of points to add. */
	void addScore(GameScore_Component & score, const int & amount) {
		score.m_score += amount * score.m_multiplier;
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Asset_Sound m_soundPop, m_soundMultiplier, m_soundMultiplierLost, m_soundLevelGained;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SCOREBOARD_S_H