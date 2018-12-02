#pragma once
#ifndef POST_PROCESSING_MODULE_H
#define POST_PROCESSING_MODULE_H

#include "Modules\Engine_Module.h"
#include "Modules\Post Processing\Effects\GFX_PP_Effect.h"
#include <vector>


class ECS;

/** A module responsible for post-processing rendering effects. */
class Post_Processing_Module : public Engine_Module {
public:
	// (de)Constructors
	~Post_Processing_Module() = default;
	Post_Processing_Module() = default;


	// Public Interface Implementation
	/** Initialize the module. */
	virtual void initialize(Engine * engine) override;


	// Public Methods
	/** Applies select post-processing effects to the screen.
	@param	deltaTime	the amount of time passed since last frame */
	void frameTick(const float & deltaTime);


private:
	// Private Attributes
	std::vector<GFX_PP_Effect*>	m_fxTechs;
};

#endif // POST_PROCESSING_MODULE_H