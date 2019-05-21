#pragma once
#ifndef PLAYERINPUT_S_H
#define PLAYERINPUT_S_H 

#include "States/GameSystemInterface.h"
#include "States/Puzzle/ECS/Board_C.h"
#include "States/Puzzle/Common_Lambdas.h"
#include "Assets/Sound.h"
#include "Utilities/ActionState.h"
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

			for (int x = 0; x < (int)board.m_player.tileSwaps.size(); ++x) {
				auto & tileSwap = board.m_player.tileSwaps[x];
				tileSwap.time += deltaTime;
				float xOffset = 2.0f * (1.0f - std::clamp<float>(tileSwap.time / Tile_SwapDuration, 0.0f, 1.0f));
				board.m_data->data->tiles[(tileSwap.yIndex * 6) + tileSwap.xIndices[0]].xOffset = xOffset;
				board.m_data->data->tiles[(tileSwap.yIndex * 6) + tileSwap.xIndices[1]].xOffset = -xOffset;
				if (tileSwap.time >= Tile_SwapDuration) {
					board.m_data->data->tiles[(tileSwap.yIndex * 6) + tileSwap.xIndices[0]].xOffset = 0.0f;
					board.m_data->data->tiles[(tileSwap.yIndex * 6) + tileSwap.xIndices[1]].xOffset = 0.0f;
					board.m_player.tileSwaps.pop_front();
					x--;
				}
			}			

			// Exit early if game hasn't started
			if (!board.m_gameInProgress)
				continue;
			
			// Move Left
			if (isAction(ActionState::LEFT)) {
				board.m_player.xPos--;
				m_engine->getManager_Sounds().playSound(m_soundMove);
			}
			// Move Right
			if (isAction(ActionState::RIGHT)) {
				board.m_player.xPos++;
				m_engine->getManager_Sounds().playSound(m_soundMove);
			}
			// Move Down
			if (isAction(ActionState::BACKWARD)) {
				board.m_player.yPos--;
				m_engine->getManager_Sounds().playSound(m_soundMove);
			}
			// Move Up
			if (isAction(ActionState::FORWARD)) {
				board.m_player.yPos++;
				m_engine->getManager_Sounds().playSound(m_soundMove);
			}
			// Swap Tiles
			if (isAction(ActionState::JUMP)) {
				if (swapTiles(std::make_pair(board.m_player.xPos, board.m_player.yPos), std::make_pair(board.m_player.xPos + 1, board.m_player.yPos), board)) {
					board.m_player.tileSwaps.push_back({ board.m_player.xPos, board.m_player.xPos + 1, board.m_player.yPos, 0.0f });
					m_engine->getManager_Sounds().playSound(m_soundSwitch, 0.5f, 1.5f);
				}
			}
			// Fast Forward
			if (isAction(ActionState::RUN)) {
				board.m_skipWaiting = true;
				board.m_rowsToAdd = 1;
				m_engine->getManager_Sounds().playSound(m_soundScroll, 0.33f);
			}

			board.m_player.xPos = std::min(4, std::max(0, board.m_player.xPos));
			board.m_player.yPos = std::min(11, std::max(1, board.m_player.yPos));
			board.m_data->data->playerCoords = glm::ivec2(board.m_player.xPos, board.m_player.yPos);
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