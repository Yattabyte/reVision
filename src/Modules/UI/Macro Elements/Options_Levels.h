#pragma once
#ifndef OPTIONS_LEVELS_H
#define OPTIONS_LEVELS_H

#include "Modules/UI/Macro Elements/Options_Pane.h"
#include "glm/glm.hpp"
#include <string>


/** A UI element serving as a level options menu. */
class Options_Levels final : public Options_Pane {
public:
	// Public Interaction Enums
	enum class Interact : int {
		on_levelSelect = (int)UI_Element::Interact::last_interact_index,
		on_back,
	};


	// Public (De)Constructors
	/** Construct a levels pane.
	@param	engine		reference to the engine to use. */
	explicit Options_Levels(Engine& engine);


	// Public Attributes
	std::string m_level;
};

#endif // OPTIONS_LEVELS_H