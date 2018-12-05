#pragma once
#ifndef TIMER_S_H
#define TIMER_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Modules\Game\Common.h"
#include "Modules\Game\Components\GameBoard_C.h"
#include "Modules\Game\Components\GameScore_C.h"


/** Responsible for updating the game timer elements. */
class Timer_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Timer_System() = default;
	Timer_System() {
		// Declare component types used
		addComponentType(GameBoard_Component::ID);
		addComponentType(GameScore_Component::ID);
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(GameBoard_Component*)componentParam[0];
			auto & score = *(GameScore_Component*)componentParam[1];

			// Tick stop-timer
			if (++score.m_stopTimeTick >= TickCount_Time) {
				score.m_stopTimeTick = 0;
				++board.m_data->data->gameTimer;
				if (score.m_stopTimer >= 0) {
					score.m_stopTimer--;
					score.m_timerAnimationTick = 0;
				}
			}

			if (score.m_stopTimer < 0 && score.m_timerAnimationTick != -1) {
				if (score.m_timerAnimationTick < TickCount_Time)
					++score.m_timerAnimationTick;
				board.m_data->data->timeAnimLinear = 1.0f - easeOutBounce(1.0f - (float(score.m_timerAnimationTick) / float(TickCount_Time)));
				if (score.m_timerAnimationTick == TickCount_Time)
					score.m_timerAnimationTick = -1;
			}

		}
	}
};

#endif // TIMER_S_H