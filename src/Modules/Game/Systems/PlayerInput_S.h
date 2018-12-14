#pragma once
#ifndef PLAYERINPUT_S_H
#define PLAYERINPUT_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Assets\Asset_Sound.h"
#include "Modules\Game\Common.h"
#include "Modules\Game\Components\GameBoard_C.h"
#include "Utilities\ActionState.h"
#include "Engine.h"


/** Responsible for interfacing the user with the game. */
class PlayerInput_System : public BaseECSSystem {
public:
	// (de)Constructors
	~PlayerInput_System() = default;
	PlayerInput_System(Engine * engine, ActionState * actionState) : m_engine(engine), m_actionState(actionState) {
		// Declare component types used
		addComponentType(GameBoard_Component::ID);

		// Asset Loading
		m_soundMove = Shared_Sound(m_engine, "Game\\move.wav");
		m_soundSwitch = Shared_Sound(m_engine, "Game\\switch.wav");
		m_soundScroll = Shared_Sound(m_engine, "Game\\scroll.wav");
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(GameBoard_Component*)componentParam[0];
			
			// Move Left
			if (m_actionState->at(ActionState::LEFT) > 0.5f) {
				if (!m_keyPressStates[ActionState::LEFT]) {
					board.m_playerX--;
					m_keyPressStates[ActionState::LEFT] = true;
					if (m_soundMove->existsYet())
						m_engine->getSoundManager().playWav(m_soundMove->m_soundObj);
				}
			}
			else
				m_keyPressStates[ActionState::LEFT] = false;
			// Move Right
			if (m_actionState->at(ActionState::RIGHT) > 0.5f) {
				if (!m_keyPressStates[ActionState::RIGHT]) {
					board.m_playerX++;
					m_keyPressStates[ActionState::RIGHT] = true;
					if (m_soundMove->existsYet())
						m_engine->getSoundManager().playWav(m_soundMove->m_soundObj);
				}
			}
			else
				m_keyPressStates[ActionState::RIGHT] = false;
			// Move Down
			if (m_actionState->at(ActionState::BACKWARD) > 0.5f) {
				if (!m_keyPressStates[ActionState::BACKWARD]) {
					board.m_playerY--;
					m_keyPressStates[ActionState::BACKWARD] = true;
					if (m_soundMove->existsYet())
						m_engine->getSoundManager().playWav(m_soundMove->m_soundObj);
				}
			}
			else
				m_keyPressStates[ActionState::BACKWARD] = false;
			// Move Up
			if (m_actionState->at(ActionState::FORWARD) > 0.5f) {
				if (!m_keyPressStates[ActionState::FORWARD]) {
					board.m_playerY++;
					m_keyPressStates[ActionState::FORWARD] = true;
					if (m_soundMove->existsYet())
						m_engine->getSoundManager().playWav(m_soundMove->m_soundObj);
				}
			}
			else
				m_keyPressStates[ActionState::FORWARD] = false;
			// Swap Tiles
			if (m_actionState->at(ActionState::JUMP) > 0.5f) {
				if (!m_keyPressStates[ActionState::JUMP]) {
					swapTiles(std::make_pair(board.m_playerX, board.m_playerY), std::make_pair(board.m_playerX + 1, board.m_playerY), board);
					m_keyPressStates[ActionState::JUMP] = true;
					if (m_soundSwitch->existsYet())
						m_engine->getSoundManager().playWav(m_soundSwitch->m_soundObj, 0.5f);
				}
			}
			else
				m_keyPressStates[ActionState::JUMP] = false;
			// Fast Forward
			if (m_actionState->at(ActionState::RUN) > 0.5f) {
				if (!m_keyPressStates[ActionState::RUN]) {
					m_keyPressStates[ActionState::RUN] = true;
					if (!board.m_stop) {
						board.m_rowsToAdd++;
						if (m_soundScroll->existsYet())
							m_engine->getSoundManager().playWav(m_soundScroll->m_soundObj, 0.33f);
					}
				}
			}
			else
				m_keyPressStates[ActionState::RUN] = false;

			board.m_playerX = std::min(4, std::max(0, board.m_playerX));
			board.m_playerY = std::min(11, std::max(1, board.m_playerY));
			board.m_data->data->playerCoords = glm::ivec2(board.m_playerX, board.m_playerY);
		}
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Sound m_soundMove, m_soundSwitch, m_soundScroll;
	ActionState * m_actionState = nullptr;
	std::map<ActionState::ACTION_ENUM, bool> m_keyPressStates;
};

#endif // PLAYERINPUT_S_H