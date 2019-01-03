#pragma once
#ifndef SCORE_C_H
#define SCORE_C_H

#include "Utilities\ECS\ecsComponent.h"
#include "Modules\Game\Common_Definitions.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"


/** Holds an int coordinate pair. */
struct XY { int x, y; };
/** Holds tile adjaceny information. */
struct TileAdj { bool scored[3][3] = { false, false, false, false, false, false, false, false, false }; };
/** A component representing a basic player. */
struct Score_Component : public ECSComponent<Score_Component> {
	VB_Element<GameBuffer> * m_data = nullptr;
	int m_score = 0;
	int m_lastScore = 0;
	int m_multiplier = 0;
	int m_timerAnimationTick = -1;
	int m_level = 1;
	int m_tilesCleared = 0;
	int m_levelUpTick = 0;
	float m_gameTimer = 0.0f;
	float m_stopTimer = -1.0f;
	float m_timerPowerOn = 0.0f;
	float m_levelLinear = 0.0f;
	float m_levelUpLinear = 0.0f;
	bool m_comboChanged = false;
	bool m_levelUp = false;
	struct ScoringData {
		std::vector<XY> xy;		
		float time;
		bool scored;
	};
	std::vector<ScoringData> m_scoredTiles;
	std::vector<std::vector<TileAdj>> m_scoredAdjacency;
};
/** A constructor to aid in creation. */
struct Score_Constructor : ECSComponentConstructor<Score_Component> {
	// (de)Constructors
	Score_Constructor(VB_Element<GameBuffer> * gameData)
		: m_gameData(gameData) {};
	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new Score_Component();
		component->m_data = m_gameData;
		return { component, component->ID };
	}
private:
	// Private Attributes
	VB_Element<GameBuffer> * m_gameData = nullptr;
};

#endif // SCORE_C_H