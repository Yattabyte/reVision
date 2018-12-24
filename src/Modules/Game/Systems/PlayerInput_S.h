#pragma once
#ifndef PLAYERINPUT_S_H
#define PLAYERINPUT_S_H 

#include "Modules\Game\Systems\Interface.h"
#include "Modules\Game\Components\Board_C.h"
#include "Modules\Game\Common_Lambdas.h"
#include "Assets\Asset_Sound.h"
#include "Utilities\ActionState.h"
#include "Engine.h"


/** Responsible for interfacing the user with the game. */
class PlayerInput_System : public Game_System_Interface {
public:
	// (de)Constructors
	~PlayerInput_System() = default;
	PlayerInput_System(Engine * engine, ActionState * actionState) : m_engine(engine), m_actionState(actionState) {
		// Declare component types used
		addComponentType(Board_Component::ID);

		// Asset Loading
		m_soundMove = Shared_Sound(m_engine, "Game\\move.wav");
		m_soundSwitch = Shared_Sound(m_engine, "Game\\switch.wav");
		m_soundScroll = Shared_Sound(m_engine, "Game\\scroll.wav");
	}


	// Interface Implementation
	virtual bool readyToUse() override {		
		return m_soundMove->existsYet() && m_soundSwitch->existsYet() && m_soundScroll->existsYet();
	}
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(Board_Component*)componentParam[0];
			
			// (un)Pause
			if (isAction(ActionState::PAUSE)) {
				board.m_intro.start = !board.m_intro.start;
			}					

			// Exit early if game hasn't started
			if (!board.m_intro.finished)
				continue;
			
			// Move Left
			if (isAction(ActionState::LEFT)) {
				board.m_playerX--;
				m_engine->getSoundManager().playSound(m_soundMove);
			}
			// Move Right
			if (isAction(ActionState::RIGHT)) {
				board.m_playerX++;
				m_engine->getSoundManager().playSound(m_soundMove);
			}
			// Move Down
			if (isAction(ActionState::BACKWARD)) {
				board.m_playerY--;
				m_engine->getSoundManager().playSound(m_soundMove);
			}
			// Move Up
			if (isAction(ActionState::FORWARD)) {
				board.m_playerY++;
				m_engine->getSoundManager().playSound(m_soundMove);
			}
			// Swap Tiles
			if (isAction(ActionState::JUMP)) {
				swapTiles(std::make_pair(board.m_playerX, board.m_playerY), std::make_pair(board.m_playerX + 1, board.m_playerY), board);
				m_engine->getSoundManager().playSound(m_soundSwitch, 0.5f, 1.5f);
			}
			// Fast Forward
			if (isAction(ActionState::RUN)) {
				board.m_skipWaiting = true;
				board.m_rowsToAdd = 1;
				m_engine->getSoundManager().playSound(m_soundScroll, 0.33f);
			}

			board.m_playerX = std::min(4, std::max(0, board.m_playerX));
			board.m_playerY = std::min(11, std::max(1, board.m_playerY));
			board.m_data->data->playerCoords = glm::ivec2(board.m_playerX, board.m_playerY);
		}
	}


private:
	// Private Functions
	bool isAction(const ActionState::ACTION_ENUM && actionEnum) {
		if (m_actionState->at(actionEnum) > 0.5f){
			if (!m_keyPressStates[actionEnum]) {
				m_keyPressStates[actionEnum] = true;
				return true;
			}
		}
		else 
			m_keyPressStates[actionEnum] = false;
		return false;
	}


	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Sound m_soundMove, m_soundSwitch, m_soundScroll;
	ActionState * m_actionState = nullptr;
	std::map<ActionState::ACTION_ENUM, bool> m_keyPressStates;
};

#endif // PLAYERINPUT_S_H