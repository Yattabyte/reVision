#pragma once
#ifndef INTROOUTRO_S_H
#define INTROOUTRO_S_H 

#include "Modules\Game\Systems\Interface.h"
#include "Modules\Game\Components\Board_C.h"
#include "Modules\Game\Common_Lambdas.h"


/** Responsible for starting and stopping the game. */
class IntroOutro_System : public Game_System_Interface {
public:
	// (de)Constructors
	~IntroOutro_System() = default;
	IntroOutro_System() {
		// Declare component types used
		addComponentType(Board_Component::ID);
	}


	// Interface Implementation
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(Board_Component*)componentParam[0];

			if (board.m_intro.start && !board.m_gameStarted) {
				board.m_intro.tick++;
				if (board.m_intro.tick >= TickCount_Intro) {
					board.m_intro.finished = true;
					board.m_gameStarted = true;
					board.m_rowsToAdd += 6;
				}
			}
			board.m_data->data->introAnimLinear = easeInBounce(float(board.m_intro.tick) / float(TickCount_Intro));
		}
	}
};

#endif // INTROOUTRO_S_H