#pragma once
#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include <vector>


class Engine;

/***/
class Graphics_Pipeline {
public:
	// Public (de)Constructors
	/** Virtual Destructor. */
	inline virtual ~Graphics_Pipeline() = default;
	/** Constructor. */
	inline Graphics_Pipeline() = default;
	/***/
	Graphics_Pipeline(Engine * engine, const std::vector<Graphics_Technique*> techniques);


	// Public Interface
	/** Apply this lighting technique.
	@param	deltaTime	the amount of time passed since last frame. */
	void render(const float & deltaTime);


protected:
	// Protected Attributes
	Engine * m_engine = nullptr;
	std::vector<Graphics_Technique*> m_techniques;
};

#endif // GRAPHICS_PIPELINE_H