#pragma once
#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "Modules/Graphics/Common/Camera.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Modules/Graphics/Geometry/Prop/Prop_Technique.h"
#include "Modules/Graphics/Lighting/Shadow/Shadow_Technique.h"
#include "Modules/Graphics/Lighting/Direct/Direct_Technique.h"
#include "Modules/Graphics/Lighting/Indirect/Indirect_Technique.h"
#include "Modules/Graphics/Lighting/Reflector/Reflector_Technique.h"
#include "Modules/Graphics/Effects/Skybox.h"
#include "Modules/Graphics/Effects/SSAO.h"
#include "Modules/Graphics/Effects/SSR.h"
#include "Modules/Graphics/Effects/Join_Reflections.h"
#include "Modules/Graphics/Effects/Bloom.h"
#include "Modules/Graphics/Effects/HDR.h"
#include "Modules/Graphics/Effects/FXAA.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/ecsWorld.h"
#include "Utilities/GL/GL_Vector.h"
#include <vector>


// Forward Declarations
class Engine;

/** Represents a series of graphics rendering techniques to apply serially. */
class Graphics_Pipeline {
public:
	// Public (De)Constructors
	/** Destroy this rendering pipeline. */
	inline ~Graphics_Pipeline() = default;
	/** Construct a PBR rendering pipeline.
	@param	engine			reference to the engine to use. 
	@param	clientCamera	the main camera. */
	Graphics_Pipeline(Engine& engine, Camera& clientCamera);


	// Public Methods
	/** Prepare the pipeline for rendering.
	@param	deltaTime		the amount of time passed since last frame.
	@param	world			the ecsWorld to source data from.
	@param	cameras			the cameras to render from.
	@return					camera and layer indices to render with. */
	std::vector<std::pair<int, int>> begin(const float& deltaTime, ecsWorld& world, std::vector<Camera>& cameras);
	/** Flush the pipeline after rendering.
	@param	deltaTime		the amount of time passed since last frame. */
	void end(const float& deltaTime);
	/** Apply this lighting technique.
	@param	deltaTime		the amount of time passed since last frame.
	@param	viewport		the viewport to render into.
	@param	perspectives	the camera and layer indices to render with.
	@param	categories		the allowed technique categories to render. */
	void render(const float& deltaTime, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives, const unsigned int& categories = (unsigned int)Graphics_Technique::Technique_Category::ALL);
	/** Use geometry techniques to cull shadows.
	@param	deltaTime		the amount of time passed since last frame.
	@param	perspectives	the camera and layer indices to render. */
	void cullShadows(const float& deltaTime, const std::vector<std::pair<int, int>>& perspectives);
	/** Use geometry techniques to render shadows.
	@param	deltaTime		the amount of time passed since last frame. */
	void renderShadows(const float& deltaTime);


protected:
	// Protected Attributes
	Engine& m_engine;
	Prop_Technique m_propView;
	Shadow_Technique m_shadowing;
	Direct_Technique m_directLighting;
	Indirect_Technique m_indirectLighting;
	Reflector_Technique m_reflectorLighting;
	Skybox m_skybox;
	SSAO m_ssao;
	SSR m_ssr;
	Join_Reflections m_joinReflections;
	Bloom m_bloom;
	HDR m_hdr;
	FXAA m_fxaa;
	Geometry_Technique* const m_geometryTechniques[1] = {&m_propView};
	Graphics_Technique* const m_lightingTechniques[5] = {&m_shadowing, &m_directLighting, &m_indirectLighting, &m_skybox, &m_reflectorLighting};
	Graphics_Technique* const m_effectTechniques[6] = {&m_ssao, &m_ssr, &m_joinReflections, &m_bloom, &m_hdr, &m_fxaa};
	Graphics_Technique* const m_allTechniques[12] = {&m_propView, &m_shadowing, &m_directLighting, &m_indirectLighting, &m_skybox, &m_reflectorLighting, &m_ssao, &m_ssr, &m_joinReflections, &m_bloom, &m_hdr, &m_fxaa };
	std::vector<Camera*> m_sceneCameras;
	GL_Vector<Camera::GPUData> m_cameraBuffer;
	ecsSystemList m_worldSystems, m_cameraSystems;
	std::shared_ptr<ecsBaseSystem> m_transHierachy;
};

#endif // GRAPHICS_PIPELINE_H