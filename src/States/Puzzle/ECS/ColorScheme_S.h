#pragma once
#ifndef COLORSCHEME_S_H
#define COLORSCHEME_S_H 

#include "States/Puzzle/GameSystemInterface.h"
#include "States/Puzzle/ECS/components.h"


/** Responsible for the overall color scheme for the game. */
class ColorScheme_System : public Game_System_Interface {
public:
	// Public (de)Constructors
	/** Destroy this puzzle color-scheme system. */
	inline ~ColorScheme_System() = default;
	/** Construct a puzzle color-scheme system. */
	inline ColorScheme_System() {
		// Declare component types used
		addComponentType(Board_Component::ID);
		addComponentType(Score_Component::ID);
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(Board_Component*)componentParam[0];
			auto & score = *(Score_Component*)componentParam[1];

			// Normal Color
			glm::vec3 color = glm::mix(glm::vec3(0, 0.5, 1), glm::vec3(1, 0, 0.5), (score.m_multiplier / 10.0f));
			// Critical Color
			if (board.m_critical)
				color = glm::vec3(1, 1, 0);
			// Level Up Color
			color = glm::mix(color, glm::vec3(0, 1, 0), score.m_levelUpLinear);
			board.m_data->data->colorScheme = glm::mix(board.m_data->data->colorScheme, color, 0.25f);
		}
	}
};

#endif // COLORSCHEME_S_H