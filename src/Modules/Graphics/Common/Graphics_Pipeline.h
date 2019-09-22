#pragma once
#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Geometry/Geometry_Technique.h"
#include "Modules/Graphics/Common/RH_Volume.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Modules/ECS/ecsSystem.h"
#include <vector>


class Engine;

/** Represents a series of graphics rendering techniques to apply serially. */
class Graphics_Pipeline {
public:
	// Public (de)Constructors
	/** Destroy this rendering pipeline. */
	inline ~Graphics_Pipeline() = default;
	/** Construct a PBR rendering pipeline.
	@param	engine			the engine to use.
	@param	cameras			all the cameras active in the scene.
	@param	auxSystems		container to add extra render-related ecs systems to. */
	Graphics_Pipeline(Engine * engine, const std::shared_ptr<Camera> & clientCamera, const std::shared_ptr<std::vector<Camera*>> & cameras, const std::shared_ptr<RH_Volume> & rhVolume, ecsSystemList & auxSystems);


	// Public Methods
	/** Prepare rendering techniques for the next frame, swapping buffers and resetting data.
	@param	deltaTime		the amount of time passed since last frame. */
	void prepareForNextFrame(const float & deltaTime);
	/** Update rendering techniques, performing any necessary pre-passes. 
	@param	deltaTime		the amount of time passed since last frame. */
	void update(const float & deltaTime);
	/** Apply this lighting technique.
	@param	deltaTime		the amount of time passed since last frame.
	@param	viewport		the viewport to render into.
	@param	camera			the camera to render with.
	@param	categories		the allowed technique categories to render. */
	void render(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::vector<std::pair<int, int>> & perspectives, const unsigned int & categories = Graphics_Technique::ALL);
	/** Use geometry techniques to cull shadows.
	@param	deltaTime		the amount of time passed since last frame.
	@param	perspectives	the camera and layer indicies to render. */
	void cullShadows(const float & deltaTime, const std::vector<std::pair<int, int>> & perspectives);
	/** Use geometry techniques to render shadows.
	@param	deltaTime		the amount of time passed since last frame. */
	void renderShadows(const float & deltaTime);


protected:
	// Protected Attributes
	Engine * m_engine = nullptr;
	std::vector<Geometry_Technique*> m_geometryTechniques;
	std::vector<Graphics_Technique*> m_lightingTechniques, m_effectTechniques, m_allTechniques;
};

#endif // GRAPHICS_PIPELINE_H