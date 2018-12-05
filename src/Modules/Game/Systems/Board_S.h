#pragma once
#ifndef BOARD_S_H
#define BOARD_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Modules\Game\Common.h"
#include "Modules\Game\Components\GameBoard_C.h"


/** Responsible for updating board visual effects based on the game state. */
class Board_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Board_System() = default;
	Board_System() {
		// Declare component types used
		addComponentType(GameBoard_Component::ID);	
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(GameBoard_Component*)componentParam[0];	
		
			// Board effects
			board.m_gameTick = ++board.m_gameTick > (TickCount_GameAnimation / (8.0f * board.m_data->data->excitementLinear)) ? 0 : board.m_gameTick;
			board.m_data->data->gameWave = 2.0f * float(board.m_gameTick) / (TickCount_GameAnimation / (8.0f * board.m_data->data->excitementLinear)) - 1.0f;
			board.m_data->data->excitementLinear = std::max(0.0f, std::min(1.0f, board.m_data->data->excitementLinear -= 0.001f));
			board.m_data->data->colorScheme = glm::mix(glm::vec3(0, 0.5, 1), glm::vec3(1, 0, 0.5), board.m_data->data->excitementLinear);			
		}
	}
};

#endif // BOARD_S_H