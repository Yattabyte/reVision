#pragma once
#ifndef GAMESCORE_C_H
#define GAMESCORE_C_H

#include "Utilities\ECS\ecsComponent.h"
#include "glm\glm.hpp"
#include <vector>


/** Holds an int coordinate pair. */
struct XY { int x, y; };
/** A component representing a basic player. */
struct GameScore_Component : public ECSComponent<GameScore_Component> {
	int m_score = 0;
	int m_lastScore = 0;
	int m_stopTimeTick = 0;
	int m_stopTimer = -1;
	int m_multiplier = 0;
	int m_timerAnimationTick = 0;
	bool m_comboChanged = false;
	std::vector<std::pair<std::vector<XY>, bool>> m_scoredTiles;
};
/** A constructor to aid in creation. */
struct GameScore_Constructor : ECSComponentConstructor<GameScore_Component> {
	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new GameScore_Component();
		return { component, component->ID };
	}
};

#endif // GAMESCORE_C_H