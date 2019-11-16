#pragma once
#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Geometry/Geometry_Technique.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/ecsWorld.h"
#include "Utilities/GL/GL_Vector.h"
#include <vector>


class Engine;

/** Represents a series of graphics rendering techniques to apply serially. */
class Graphics_Pipeline {
public:
	// Public (De)Constructors
	/** Destroy this rendering pipeline. */
	inline ~Graphics_Pipeline() = default;
	/** Construct a PBR rendering pipeline.
	@param	engine			the engine to use.
	@param	clientCamera	the main camera.
	@param	cameras			all the cameras active in the scene.
	@param	auxSystems		container to add extra render-related ecsSystem's to. */
	Graphics_Pipeline(Engine* engine, const std::shared_ptr<Camera>& clientCamera) noexcept;


	// Public Methods
	/** Prepare the pipeline for rendering.
	@param	deltaTime		the amount of time passed since last frame.
	@param	world			the ecsWorld to source data from.
	@param	cameras			the cameras to render from.
	@return					camera and layer indices to render with. */
	std::vector<std::pair<int, int>> begin(const float& deltaTime, ecsWorld& world, const std::vector<std::shared_ptr<Camera>>& cameras = {}) noexcept;
	/** Flush the pipeline after rendering.
	@param	deltaTime		the amount of time passed since last frame. */
	void end(const float& deltaTime) noexcept;
	/** Apply this lighting technique.
	@param	deltaTime		the amount of time passed since last frame.
	@param	viewport		the viewport to render into.
	@param	perspectives	the camera and layer indices to render with.
	@param	categories		the allowed technique categories to render. */
	void render(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives, const unsigned int& categories = (unsigned int)Graphics_Technique::Technique_Category::ALL) noexcept;
	/** Use geometry techniques to cull shadows.
	@param	deltaTime		the amount of time passed since last frame.
	@param	perspectives	the camera and layer indices to render. */
	void cullShadows(const float& deltaTime, const std::vector<std::pair<int, int>>& perspectives) noexcept;
	/** Use geometry techniques to render shadows.
	@param	deltaTime		the amount of time passed since last frame. */
	void renderShadows(const float& deltaTime) noexcept;


protected:
	// Protected Attributes
	Engine* m_engine = nullptr;
	std::shared_ptr<std::vector<Camera*>> m_sceneCameras;
	GL_Vector<Camera::GPUData> m_cameraBuffer;
	ecsSystemList m_worldSystems, m_cameraSystems;
	std::shared_ptr<ecsBaseSystem> m_transHierachy;
	std::vector<Geometry_Technique*> m_geometryTechniques;
	std::vector<Graphics_Technique*> m_lightingTechniques, m_effectTechniques, m_allTechniques;
};

#endif // GRAPHICS_PIPELINE_H
