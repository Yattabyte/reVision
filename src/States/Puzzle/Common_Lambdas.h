#pragma once
#ifndef COMMON_LAMBDAS_H
#define COMMON_LAMBDAS_H 

#include "States/Puzzle/ECS/components.h"


// Easing Functions
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
static constexpr auto easeInBounce = [](float t) {
	return 1.0f - easeOutBounce(1.0f - t);
};
static constexpr auto smoothStart2 = [](const float & t) {
	return t * t;
};
static constexpr auto smoothStart3 = [](const float & t) {
	return t * t * t;
};
static constexpr auto smoothStart4 = [](const float & t) {
	return t * t * t * t;
};
static constexpr auto smoothStart5 = [](const float & t) {
	return t * t * t * t * t;
};
/** Swap 2 tiles ONLY if they're active, not falling, and not scored
	@param		tile1		the first tile, swaps with the second.
	@param		tile2		the second tile, swaps with the first. */
static constexpr auto swapTiles = [](const auto & coordsA, const auto & coordsB, Board_Component & board) {
	auto & tileState1 = board.m_tiles[coordsA.second][coordsA.first];
	auto & tileState2 = board.m_tiles[coordsB.second][coordsB.first];
	auto & tileDrop1 = board.m_tileDrops[coordsA.second][coordsA.first];
	auto & tileDrop2 = board.m_tileDrops[coordsB.second][coordsB.first];
	if (tileState1.m_scoreType != TileState::UNMATCHED || tileState2.m_scoreType != TileState::UNMATCHED ||
		tileDrop1.dropState == Board_Component::TileDropData::FALLING || tileDrop2.dropState == Board_Component::TileDropData::FALLING)
		return false;

	// Swap mechanism
	auto copyState = tileState1;
	tileState1 = tileState2;
	tileState2 = copyState;
	auto copyDrop = tileDrop1;
	tileDrop1 = tileDrop2;
	tileDrop2 = copyDrop;
	return true;
};

#endif // COMMON_LAMBDAS_H