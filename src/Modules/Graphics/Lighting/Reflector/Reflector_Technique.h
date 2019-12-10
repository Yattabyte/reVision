#pragma once
#ifndef REFLECTOR_TECHNIQUE_H
#define REFLECTOR_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Pipeline.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"
#include "Modules/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/StaticMultiBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"


/** A core lighting technique responsible for all parallax reflectors. */
class Reflector_Technique final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destroy this reflector technique. */
	~Reflector_Technique() noexcept;
	/** Construct a reflector technique.
	@param	engine			reference to the engine to use. 
	@param	sceneCameras	reference to the scene cameras to use. */
	Reflector_Technique(Engine& engine, std::vector<Camera*>& sceneCameras) noexcept;


	// Public Interface Implementations
	void clearCache(const float& deltaTime) noexcept final;
	void updateCache(const float& deltaTime, ecsWorld& world) noexcept final;
	void updatePass(const float& deltaTime) noexcept final;
	void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept final;


private:
	// Private Methods
	/** Render all the geometry for each reflector.
	@param	deltaTime	the amount of time passed since last frame. */
	void updateReflectors(const float& deltaTime) noexcept;
	/** Render all the lights
	@param	deltaTime	the amount of time passed since last frame.
	@param	viewport	the viewport to render from. */
	void renderReflectors(const float& deltaTime, const std::shared_ptr<Viewport>& viewport) noexcept;


	// Private Attributes
	Engine& m_engine;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	Shared_Shader m_shaderLighting, m_shaderStencil, m_shaderCopy, m_shaderConvolute;
	Shared_Auto_Model m_shapeCube, m_shapeQuad;
	StaticMultiBuffer<> m_indirectQuad = StaticMultiBuffer(sizeof(GLuint) * 4), m_indirectQuadConvolute = StaticMultiBuffer(sizeof(GLuint) * 4);
	std::shared_ptr<Viewport> m_viewport;
	struct DrawData {
		DynamicBuffer<> bufferCamIndex;
		DynamicBuffer<> visLights;
		StaticMultiBuffer<> indirectShape = StaticMultiBuffer(sizeof(GLuint) * 4);
	};
	int m_drawIndex = 0;
	std::vector<DrawData> m_drawData;
	ecsSystemList m_auxilliarySystems;


	// Shared Attributes
	ReflectorData m_frameData;
	std::vector<Camera*>& m_sceneCameras;
};

#endif // REFLECTOR_TECHNIQUE_H