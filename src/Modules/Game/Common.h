#pragma once
#ifndef COMMON_H
#define COMMON_H 

constexpr unsigned int BOARD_WIDTH = 6;
constexpr unsigned int BOARD_HEIGHT = 12;
constexpr float TickCount_GameAnimation		= 750.0f;
constexpr int	TickCount_NewLine			= 500u;
constexpr int	TickCount_Time				= 100;
constexpr float TickCount_TileDrop			= 10.0F;
constexpr float TickCount_TileBounce		= 15.0F;
constexpr int	TickCount_Scoring			= 50;
constexpr int	TickCount_Popping			= 25;
constexpr int	TickCount_LevelUp			= 75;

// Bouncing Easing Function
static constexpr auto easeOutBounce = [](float t) {
	if (t < (1.0f / 2.75f))
		return (7.5625f * t *t);
	else if (t < (2.0f / 2.75f))
		return (7.5625f * (t -= (1.5f / 2.75f)) * t + .75f);
	else if (t < (2.5f / 2.75f))
		return (7.5625f * (t -= (2.25f / 2.75f)) * t + .9375f);
	else
		return (7.5625f * (t -= (2.625f / 2.75f)) * t + .984375f);
};

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

#endif // COMMON_H