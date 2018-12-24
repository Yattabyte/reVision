#pragma once
#ifndef COLORSCHEME_S_H
#define COLORSCHEME_S_H 

#include "Modules\Game\Systems\Interface.h"
#include "Modules\Game\Components\Board_C.h"
#include "Modules\Game\Components\Score_C.h"


/** Responsible for the overall color scheme for the game. */
class ColorScheme_System : public Game_System_Interface {
public:
	// (de)Constructors
	~ColorScheme_System() = default;
	ColorScheme_System() {
		// Declare component types used
		addComponentType(Board_Component::ID);
		addComponentType(Score_Component::ID);
	}


	// Interface Implementation
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(Board_Component*)componentParam[0];
			auto & score = *(Score_Component*)componentParam[1];

			// Normal Color
			glm::vec3 color = glm::mix(glm::vec3(0, 0.5, 1), glm::vec3(1, 0, 0.5), board.m_data->data->excitementLinear);
			// Critical Color
			if (board.m_nearingTop)
				color = glm::vec3(1, 1, 0);
			// Level Up Color
			color = glm::mix(color, glm::vec3(0, 1, 0), score.m_levelUpLinear);
			board.m_data->data->colorScheme = glm::mix(board.m_data->data->colorScheme, color, 0.25f);
		}
	}
};

#endif // COLORSCHEME_S_H