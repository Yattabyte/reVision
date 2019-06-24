#pragma once
#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Common/Viewport.h"
#include <vector>


class Engine;

/** Represents a series of graphics rendering techniques to apply serially. */
class Graphics_Pipeline {
public:
	// Public (de)Constructors
	/** Destroy this rendering pipeline. */
	inline ~Graphics_Pipeline() = default;
	/** Construct a PBR rendering pipeline. 
	@param	engine		the engine to use.
	@param	auxSystems	container to add extra render-related ecs systems to. */
	Graphics_Pipeline(Engine * engine, ECSSystemList & auxSystems);


	// Public Methods
	/***/
	void beginFrame(const float & deltaTime);
	/***/
	void endFrame(const float & deltaTime);
	/***/
	void update(const float & deltaTime);
	/** Apply this lighting technique.
	@param	deltaTime	the amount of time passed since last frame. 
	@param	viewport	the viewport to render into.
	@param	categories	the allowed technique categories to render. */
	void render(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const unsigned int & categories = Graphics_Technique::ALL );


protected:
	// Protected Attributes
	Engine * m_engine = nullptr;
	std::vector<Graphics_Technique*> m_techniques;
};

#endif // GRAPHICS_PIPELINE_H