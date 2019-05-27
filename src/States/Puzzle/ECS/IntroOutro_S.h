#pragma once
#ifndef INTROOUTRO_S_H
#define INTROOUTRO_S_H 

#include "States/GameSystemInterface.h"
#include "States/Puzzle/ECS/components.h"
#include "States/Puzzle/Common_Lambdas.h"
#include "Assets/Sound.h"
#include "Engine.h"


/** Responsible for starting and stopping the game. */
class IntroOutro_System : public Game_System_Interface {
public:
	// Public (de)Constructors
	/** Destroy this puzzle intro/outro system. */
	~IntroOutro_System() = default;
	/** Construct a puzzle intro/outro system. */
	IntroOutro_System(Engine * engine) : m_engine(engine) {
		// Declare component types used
		addComponentType(Board_Component::ID);
		m_soundBeep = Shared_Sound(engine, "Game\\beep.wav");
		m_soundBeepEnd = Shared_Sound(engine, "Game\\beep finish.wav");
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(Board_Component*)componentParam[0];

			board.m_data->data->intro.countDown = -1;
			if (!board.m_gameInProgress && !board.m_intro.finished) {
				board.m_data->data->intro.powerSecondary = 0.0f;
				board.m_intro.time -= deltaTime;
				if (board.m_intro.time <= 1.0f) {
					board.m_intro.finished = true;
					board.m_gameInProgress = true;
					board.m_player.xPos = 2;
					board.m_player.yPos = 0;
					board.m_rowsToAdd += 6;
				}
				else {
					const int number = 4 - int(board.m_intro.time);
					if (board.m_intro.countDown != number && number < 4)
						if (number < 3)
							m_engine->getManager_Sounds().playSound(m_soundBeep);
						else
							m_engine->getManager_Sounds().playSound(m_soundBeepEnd);
					board.m_intro.countDown = number;
					board.m_data->data->intro.countDown = number;
					board.m_data->data->intro.powerSecondary = easeInBounce(std::clamp<float>((4.0f - board.m_intro.time) / 2.0f, 0.0f, 1.0f));
				}
			}
			else 
				board.m_data->data->intro.powerSecondary = 1.0f;	
			if (board.m_gameEnded && !board.m_outro.finished) {
				if (board.m_outro.time < 1.0f) {
					board.m_outro.time += deltaTime;
					const float life = board.m_outro.time / 1.0f;
					for (int x = 0; x < 6; ++x) {
						board.m_data->data->lanes[x] = 0.0f;
						for (int y = 0; y < 12; ++y) 
							board.m_data->data->tiles[(y * 6) + x].lifeLinear = life;
					}

				}
				else {
					board.m_outro.finished = true;
					for (int x = 0; x < 6; ++x)
						for (int y = 0; y < 12; ++y) {
							board.m_tiles[y][x].m_type = TileState::NONE;
							board.m_tiles[y][x].m_scoreType = TileState::UNMATCHED;
							board.m_data->data->tiles[(y * 6) + x].lifeLinear = 0.0f;
							board.m_data->data->tiles[(y * 6) + x].type = board.m_tiles[y][x].m_type;
						}
				}
			}
			board.m_data->data->intro.powerOn = easeInBounce(std::clamp<float>((6.0f - board.m_intro.time) / 4.0f, 0.0f, 1.0f));
		}
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Sound m_soundBeep, m_soundBeepEnd;
};

#endif // INTROOUTRO_S_H