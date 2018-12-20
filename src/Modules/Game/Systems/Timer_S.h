#pragma once
#ifndef TIMER_S_H
#define TIMER_S_H 

#include "Modules\Game\Systems\Interface.h"
#include "Modules\Game\Components\Score_C.h"
#include "Modules\Game\Common_Lambdas.h"


/** Responsible for updating the game timer elements. */
class Timer_System : public Game_System_Interface {
public:
	// (de)Constructors
	~Timer_System() = default;
	Timer_System() {
		// Declare component types used
		addComponentType(Score_Component::ID);
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & score = *(Score_Component*)componentParam[0];

			// Tick stop-timer
			if (++score.m_stopTimeTick >= TickCount_Time) {
				score.m_stopTimeTick = 0;
				++score.m_data->data->gameTimer;
				if (score.m_stopTimer >= 0) {
					score.m_stopTimer--;
					score.m_timerAnimationTick = 0;
				}
			}

			if (score.m_stopTimer < 0 && score.m_timerAnimationTick != -1) {
				if (score.m_timerAnimationTick < TickCount_Time)
					++score.m_timerAnimationTick;
				score.m_data->data->timeAnimLinear = 1.0f - easeOutBounce(1.0f - (float(score.m_timerAnimationTick) / float(TickCount_Time)));
				if (score.m_timerAnimationTick == TickCount_Time)
					score.m_timerAnimationTick = -1;
			}

		}
	}
};

#endif // TIMER_S_H