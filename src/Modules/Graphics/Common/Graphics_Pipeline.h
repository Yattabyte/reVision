#pragma once
#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Geometry/Geometry_Technique.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Modules/World/ECS/ecsSystem.h"
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
	@param	cameras		all the cameras active in the scene.
	@param	auxSystems	container to add extra render-related ecs systems to. */
	Graphics_Pipeline(Engine * engine, const std::shared_ptr<CameraBuffer> & clientCamera, const std::shared_ptr<std::vector<std::shared_ptr<CameraBuffer>>> & cameras, ECSSystemList & auxSystems);


	// Public Methods
	/***/
	void prepareForNextFrame(const float & deltaTime);
	/***/
	void update(const float & deltaTime);
	/** Apply this lighting technique.
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render into.
	@param	camera		the camera to render with.
	@param	categories	the allowed technique categories to render. */
	void render(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::shared_ptr<CameraBuffer> & camera, const unsigned int & categories = Graphics_Technique::ALL);
	/***/
	void shadow(const float & deltaTime, const std::shared_ptr<CameraBuffer> & camera, const int & layer, const glm::vec3 & finalColor);


protected:
	// Protected Attributes
	Engine * m_engine = nullptr;
	std::vector<Geometry_Technique*> m_geometryTechniques;
	std::vector<Graphics_Technique*> m_lightingTechniques, m_effectTechniques, m_allTechniques;
};

#endif // GRAPHICS_PIPELINE_H