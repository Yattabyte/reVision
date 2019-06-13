#pragma once
#pragma once
#ifndef SCORE_S_H
#define SCORE_S_H 

#include "States/Puzzle/GameSystemInterface.h"
#include "States/Puzzle/ECS/components.h"
#include "States/Puzzle/Common_Lambdas.h"
#include "Assets/Sound.h"
#include "Engine.h"


/** Responsible for validating the game state, checking for scoreable events. */
class Score_System : public Game_System_Interface {
public:
	// Public (de)Constructors
	/** Destroy this puzzle score system. */
	inline ~Score_System() = default;
	/** Construct a puzzle score system. */
	inline Score_System(Engine * engine) : m_engine(engine) {
		// Declare component types used
		addComponentType(Board_Component::ID);
		addComponentType(Score_Component::ID);		

		// Asset Loading
		m_soundPop = Shared_Sound(m_engine, "Game\\pop.wav");
		m_soundMultiplierInc = Shared_Sound(m_engine, "Game\\multiplier.wav");
		m_soundMultiplierLost = Shared_Sound(m_engine, "Game\\multiplier lost.wav");
		m_soundLevelGained = Shared_Sound(m_engine, "Game\\level gain.wav");
	}

	
	// Public Interface Implementation	
	inline virtual bool readyToUse() override {
		return
			m_soundPop->existsYet() &&
			m_soundMultiplierInc->existsYet() &&
			m_soundMultiplierLost->existsYet() &&
			m_soundLevelGained->existsYet();
	}
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(Board_Component*)componentParam[0];
			auto & score = *(Score_Component*)componentParam[1];

			// Exit early if game hasn't started
			if (!board.m_gameInProgress)
				continue;

			validateBoard(board, score);
			if (board.m_music.beat) {
				checkCombo(board, score);
				scoreTiles(board, score);
			}
			popTiles(deltaTime, board, score);

			// Animate score climbing
			if ((score.m_score - score.m_data->score) > 0) 
				score.m_data->score = std::min(score.m_score, score.m_data->score + 1);
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
			score.m_data->scoreAnimLinear = score.m_multiplier > 1 ? std::max(0.0f, std::min(1.0f, score.m_data->scoreAnimLinear)) : 0.0f;
			
			// Synchronize component data to GPU
			board.m_stop = bool(score.m_scoredTiles.size() || score.m_timerStop > -1.0f);
			score.m_data->shakeLinear = std::max(0.0f, std::min(1.0f, score.m_data->shakeLinear - 0.01f));
			score.m_data->highlightIndex = scoreLength - (8 - firstMostDigit);
			score.m_data->multiplier = score.m_multiplier;

			// If enough tiles cleared, signal to start the level-up animation
			if (score.m_tilesCleared >= (score.m_level * 12)) {
				score.m_level++;
				score.m_levelUp = true; 
				score.m_timerLevelUp = 0.0f;
				score.m_tilesCleared = 0;
				m_engine->getManager_Sounds().playSound(m_soundLevelGained, 0.75f);
			}
			if (score.m_levelUp) {
				score.m_timerLevelUp += deltaTime;
				if (score.m_timerLevelUp >= Game_LevelUpDuration) {
					score.m_levelUp = false;
					score.m_timerLevelUp = 0.0f;
				}
			}

			score.m_levelLinear = float(score.m_tilesCleared) / float(score.m_level * 12);			
			score.m_levelUpLinear = sinf(float(score.m_timerLevelUp) / float(Game_LevelUpDuration) * glm::pi<float>());
			board.m_speed = 10.0f - (float(score.m_level - 1));
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
		inline static bool sortFunc(const XY & a, const XY & b) {
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
	inline std::vector<TileAdj> getAdjacency(const std::vector<XY> & tileSet) {
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
	inline bool checkSeries_Horizontally(const Board_Component & board, ScoringManifold & series) {
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
	inline bool checkSeries_Vertically(const Board_Component & board, ScoringManifold & series) {
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
	inline void validateBoard(Board_Component & board, Score_Component & score) {
		std::vector<ScoringManifold> allMatchingSets;
		static constexpr auto findScoredTiles = [](const auto & xCoord, const auto & yCoord, Score_Component & score) {
			for each (const auto & data in score.m_scoredTiles)
				for each (const auto & xy in data.xy)
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
			// All matches formed in a single update tick count as a single mega set, sized as the sum of all their tiles
			// Combine their sets into 1 manifold
			ScoringManifold combinedManifold;
			for each (const auto & matchingSet in allMatchingSets)
				for each (const auto & xy in matchingSet)
					combinedManifold.push_back(xy);
			combinedManifold.sort();
			score.m_scoredTiles.push_back({
				combinedManifold, 
				false
			});
			score.m_scoredAdjacency.push_back(getAdjacency(combinedManifold));
			score.m_comboChanged = true;
		}
	}
	/** Check if we should change the combo multiplier.
	@param		board		the board containing the tiles of interest.
	@param		score		the score component. */
	inline void checkCombo(Board_Component & board, Score_Component & score) {
		// Check to see if we should increment combo multiplier and add points
		if (score.m_scoredTiles.size()) {
			if (score.m_comboChanged) {
				score.m_comboChanged = false;
				addScore(score, 5);

				score.m_multiplier++;
				score.m_timerStop++;
				score.m_data->shakeLinear += (score.m_multiplier / 5.0f);
				score.m_data->scoreAnimLinear++;

				m_engine->getManager_Sounds().playSound(m_soundMultiplierInc, 0.75f, 1.0f + (score.m_multiplier / 10.0f));
			}
		}
		// Reset the combo on the music beat, if the combo hasn't changed
		else if (!score.m_comboChanged && score.m_multiplier) {
				// Reset multiplier if no tiles are scored
				if (score.m_multiplier >= 2)
					m_engine->getManager_Sounds().playSound(m_soundMultiplierLost, 1.0f);
				score.m_multiplier = 0;
				score.m_comboChanged = false;
			}
	}
	/** Find new scoring manifolds, give points for them, and start ticking the tiles (for popping them)
	@brief		Rules:
				- Every tile matched		+pts(10) each
					(Ex: 3 tiles += 30 pts)
				- Every tile after 3rd in set +pts(10) each, +pts(setSize)
					(Ex: 5 tiles += 25 pts)
				- For every chained combo	-> +pts(5), +multiplier(1)
				- All points added are multiplied by the combo multiplier
	@param		board		the board containing the tiles of interest.
	@param		score		the score component. */
	inline void scoreTiles(Board_Component & board, Score_Component & score) {		
		// Manage scoring manifolds
		for (size_t x = 0; x < score.m_scoredTiles.size(); ++x) {
			auto & manifold = score.m_scoredTiles[x];
			// If this manifold hasn't been processed for scoring yet
			if (!manifold.scored) {
				manifold.scored = true;
				manifold.time = 0.0f;
				for (size_t tile = 0; tile < manifold.xy.size(); ++tile) {
					// Set as matched
					const auto & xy = manifold.xy[tile];
					board.m_tiles[xy.y][xy.x].m_scoreType = TileState::MATCHED;
				}
				// Add time, but never move timer past 3 seconds
				score.m_timerStop = std::min<float>(score.m_timerStop + 1.0f, 3.0f);
				// Add another 10 bonus points for every extra tile past 3, plus a base amount of 10, also add time
				if (manifold.xy.size() > 3) {
					addScore(score, int(manifold.xy.size()) + (10 * (int(manifold.xy.size()) - 3)));
					score.m_data->shakeLinear += std::max(0.25f, manifold.xy.size() / 9.0f);
					score.m_timerStop++;
				}
			}
		}
	}
	inline void popTiles(const float & deltaTime, Board_Component & board, Score_Component & score) {
		// Manage scoring manifolds
		for (size_t x = 0; x < score.m_scoredTiles.size(); ++x) {
			auto & manifold = score.m_scoredTiles[x];
			if (manifold.scored) {
				// Tick all matched tiles until they pop
				const float beatDuration = board.m_music.beatSeconds / 4.0f;
				float & time = manifold.time;
				time += deltaTime;
				for (size_t tile = 0; tile < manifold.xy.size(); ++tile) {
					const auto & xy = manifold.xy[tile];
					const float duration = (tile + 1) * beatDuration;
					const auto & x = xy.x;
					const auto & y = xy.y;

					if (board.m_tiles[y][x].m_scoreType == TileState::MATCHED) {
						// Tile SCORED
						if (time >= duration) {
							m_engine->getManager_Sounds().playSound(m_soundPop, 1.0f, 0.75f + ((tile / 10.0f) * 2.0f));
							board.m_tiles[y][x].m_scoreType = TileState::SCORED;
							addScore(score, 10);
							score.m_tilesCleared++;
						}
						score.m_data->tiles[(y * 6) + x].lifeLinear = smoothStart5(glm::clamp(time / duration, 0.0f, 1.0f));
					}
				}
			}
		}

		// Check if all tiles in a scoring set have been popped
		// If so, empty and reset them
		for (size_t x = 0; x < score.m_scoredTiles.size(); ++x) {
			auto & pair = score.m_scoredTiles[x];
			bool allPopped = true;
			for each (const auto & xy in pair.xy)
				if (board.m_tiles[xy.y][xy.x].m_scoreType != TileState::SCORED) {
					allPopped = false;
					break;
				}
			if (allPopped) {
				// Reset the tiles and make them background spaces
				for each (const auto & xy in pair.xy) {
					board.m_tiles[xy.y][xy.x].m_type = TileState::NONE;
					board.m_tiles[xy.y][xy.x].m_scoreType = TileState::UNMATCHED;
					score.m_data->tiles[(xy.y * 6) + xy.x].lifeLinear = 0.0f;
				}
				score.m_scoredTiles.erase(score.m_scoredTiles.begin() + x);
				score.m_scoredAdjacency.erase(score.m_scoredAdjacency.begin() + x);
				x--;
			}
		}
	}
	/** A wrapper function for adding points to the score.score. Accounts for any other variables like multipliers.
	@param	score	the score component.
	@param	amount	the amount of points to add. */
	inline void addScore(Score_Component & score, const int & amount) {
		score.m_score += amount * score.m_multiplier;
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Sound m_soundPop, m_soundMultiplierInc, m_soundMultiplierLost, m_soundLevelGained;
};

#endif // SCORE_S_H