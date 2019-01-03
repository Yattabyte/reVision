#pragma once
#ifndef INTROOUTRO_S_H
#define INTROOUTRO_S_H 

#include "Modules\Game\Systems\Interface.h"
#include "Modules\Game\Components\Board_C.h"
#include "Modules\Game\Common_Lambdas.h"
#include "Assets\Asset_Sound.h"
#include "Engine.h"


/** Responsible for starting and stopping the game. */
class IntroOutro_System : public Game_System_Interface {
public:
	// (de)Constructors
	~IntroOutro_System() = default;
	IntroOutro_System(Engine * engine) : m_engine(engine) {
		// Declare component types used
		addComponentType(Board_Component::ID);
		m_soundBeep = Shared_Sound(engine, "Game\\beep.wav");
		m_soundBeepEnd = Shared_Sound(engine, "Game\\beep finish.wav");
	}


	// Interface Implementation
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(Board_Component*)componentParam[0];

			board.m_data->data->intro.countDown = -1;
			if (!board.m_gameStarted) {
				board.m_data->data->intro.powerSecondary = 0.0f;
				if (board.m_intro.start ) {
					board.m_intro.time -= deltaTime;
					if (board.m_intro.time <= 1.0f) {
						board.m_intro.finished = true;
						board.m_gameStarted = true;
						board.m_playerX = 2;
						board.m_playerY = 0;
						board.m_rowsToAdd += 6;
					}
					else {
						const int number = 4 - int(board.m_intro.time);
						if (board.m_intro.countDown != number && number < 4)
							if (number < 3)
								m_engine->getSoundManager().playSound(m_soundBeep);
							else
								m_engine->getSoundManager().playSound(m_soundBeepEnd);
						board.m_intro.countDown = number;
						board.m_data->data->intro.countDown = number;
						board.m_data->data->intro.powerSecondary = easeInBounce(std::clamp<float>((4.0f - board.m_intro.time) / 2.0f, 0.0f, 1.0f));
					}
				}
			}
			else
				board.m_data->data->intro.powerSecondary = 1.0f;			
			board.m_data->data->intro.powerOn = easeInBounce(std::clamp<float>((6.0f - board.m_intro.time) / 4.0f, 0.0f, 1.0f));
		}
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Sound m_soundBeep, m_soundBeepEnd;
};

#endif // INTROOUTRO_S_H