#pragma once
#ifndef SCOREBOARD_S_H
#define SCOREBOARD_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Engine.h"
#include <algorithm>
#include <iostream>

/** Component Types Used */
#include "Modules\Game\Components\GameBoard_C.h"
#include "Modules\Game\Components\GameScore_C.h"


constexpr int TickCount_Scoring = 50;
constexpr int TickCount_Popping = 15;
constexpr int TickCount_ScoreRotate = 750;

/** A system that updates the rendering state for spot lighting, using the ECS system. */
class Score_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Score_System() = default;
	Score_System() {
		// Declare component types used
		addComponentType(GameBoard_Component::ID);
		addComponentType(GameScore_Component::ID);		
	}

	
	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(GameBoard_Component*)componentParam[0];
			auto & score = *(GameScore_Component*)componentParam[1];

			validateBoard(board, score);	
			scoreTiles(board, score);

			if ((score.m_score - board.m_data->data->score) > 0)
				board.m_data->data->score++;
			else
				score.m_lastScore = score.m_score;
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

			// Logic for animating the score
			board.m_data->data->highlightIndex = scoreLength - (8 - firstMostDigit);// std::max(0, firstMostDigit - 1);
			board.m_data->data->scoreTick++;
			if (board.m_data->data->scoreTick > TickCount_ScoreRotate)
				board.m_data->data->scoreTick = 0;
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
	/** Iterates over a scoring manifold, checking if any of its tiles are involved in any horizontal matches.
	@param		board		the board containing the tiles of interest.
	@param		series		the scoring manifold referencing the active tiles to check againts.
	@return					true if any further matches are found, false otherwise. */
	bool checkSeries_Horizontally(const GameBoard_Component & board, ScoringManifold & series) {
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
	bool checkSeries_Vertically(const GameBoard_Component & board, ScoringManifold & series) {
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
	@param		board		the board containing the tiles of interest.
	@param		score		the scoring component. */
	void validateBoard(GameBoard_Component & board, GameScore_Component & score) {
		std::vector<ScoringManifold> allMatchingSets;
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
					allMatchingSets.push_back(matchingSet);
					// Set the tiles as matched so that they aren't matched against more than once
					for each (const auto & xy in matchingSet)
						board.m_tiles[xy.y][xy.x].m_scoreType = TileState::MATCHED;
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
			}
		}
		else if (!score.m_scoredTiles.size() || !score.m_comboChanged){
			// Reset multiplier if no tiles are scored
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
				board.m_data->data->excitement += (0.075f * (float)manifold.first.size());
				// Add time, but never move timer past 3 seconds
				score.m_stopTimer = std::min(score.m_stopTimer + 1, 3);
				// Add another 10 bonus points for every extra tile past 3, plus a base amount of 10, also add time
				if (manifold.first.size() > 3) {
					addScore(score, manifold.first.size() + (10 * (manifold.first.size() - 3)));
					score.m_stopTimer += (manifold.first.size() - 3);
				}

				score.m_stopTimeTick = 0;
			}
		}


		// Tick all matched tiles until they pop
		for (int y = 1; y < 12; ++y)
			for (int x = 0; x < 6; ++x) {
				auto & xTile = board.m_tiles[y][x];
				if (xTile.m_scoreType != TileState::UNMATCHED) {
					if (xTile.m_scoreType == TileState::MATCHED) {
						if (xTile.m_tick >= TickCount_Popping) {
							addScore(score, 10);
							xTile.m_scoreType = TileState::SCORED;
						}
					}
					board.m_data->data->lifeTick[(y * 6) + x] = ++xTile.m_tick;
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
				// Reset the tiles, make them background spaces
				for each (const auto & xy in pair.first) {
					board.m_tiles[xy.y][xy.x].m_type = TileState::NONE;
					board.m_tiles[xy.y][xy.x].m_scoreType = TileState::UNMATCHED;
					board.m_tiles[xy.y][xy.x].m_tick = 0;
					board.m_data->data->lifeTick[(xy.y * 6) + xy.x] = 0.0f;
				}
				score.m_scoredTiles.erase(score.m_scoredTiles.begin() + x);
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
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SCOREBOARD_S_H