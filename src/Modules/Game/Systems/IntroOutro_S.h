#pragma once
#ifndef INTROOUTRO_S_H
#define INTROOUTRO_S_H 

#include "Modules\Game\Systems\Interface.h"
#include "Modules\Game\Components\Board_C.h"


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

			// Exit early if intro hasn't started
			if (!board.m_introStarted)
				continue;

			// Intro effects
			if (board.m_introTick <= TickCount_Intro) {
				board.m_introTick++;
				board.m_gameStarted = false;
			}
			else
				if (board.m_introStarted && !board.m_gameStarted) {
					board.m_introStarted = false;
					board.m_gameStarted = true; 
				}
			
			board.m_data->data->introAnimLinear = float(board.m_introTick) / float(TickCount_Intro);
		}
	}
};

#endif // INTROOUTRO_S_H