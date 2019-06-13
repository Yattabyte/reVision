#pragma once
#ifndef TIMER_S_H
#define TIMER_S_H 

#include "States/Puzzle/GameSystemInterface.h"
#include "States/Puzzle/ECS/components.h"
#include "States/Puzzle/Common_Lambdas.h"


/** Responsible for updating the game timer elements. */
class Timer_System : public Game_System_Interface {
public:
	// Public (de)Constructors
	/** Destroy this puzzle timer system. */
	inline ~Timer_System() = default;
	/** Construct a puzzle timer system. */
	inline Timer_System() {
		// Declare component types used
		addComponentType(Board_Component::ID);
		addComponentType(Score_Component::ID);
	}


	// Public Interface Implementation	
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(Board_Component*)componentParam[0];
			auto & score = *(Score_Component*)componentParam[1];

			// Exit early if game hasn't started
			if (!board.m_gameInProgress)
				continue;

			// Game Time Logic
			score.m_data->timeAnimLinear = 1.0f;

			if (score.m_timerStop > -1.0f) {
				score.m_timerStop -= deltaTime;
				score.m_timerPowerOn = 1.0f;
			}
			else {
				score.m_timerGame += deltaTime;
				if (score.m_timerPowerOn > 0.0f) {
					score.m_timerPowerOn -= deltaTime;
					score.m_data->timeAnimLinear = easeInBounce(1.0f - score.m_timerPowerOn);
				}
			}

			score.m_timerStop = std::clamp<float>(score.m_timerStop, -1.0f, 9.0f);
			score.m_data->stopTimer = score.m_timerStop;
			score.m_data->gameTimer = score.m_timerGame;
		}
	}
};

#endif // TIMER_S_H