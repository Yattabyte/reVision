#pragma once
#ifndef DIRECT_TECHNIQUE_H
#define DIRECT_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/Direct/DirectData.h"
#include "Modules/Graphics/Lighting/Direct/DirectVisibility_System.h"
#include "Modules/Graphics/Lighting/Direct/DirectSync_System.h"
#include "Modules/ECS/ecsSystem.h"
#include "Assets/Shader.h"
#include "Assets/Mesh.h"
#include "Utilities/GL/StaticMultiBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Engine.h"
#include <random>


/** A core lighting technique responsible for direct-diffuse and direct-specular lighting. */
class Direct_Technique final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destroy this direct lighting technique. */
	~Direct_Technique() noexcept;
	/** Construct a direct lighting technique.
	@param	engine			reference to the engine to use. 
	@param	shadowData		reference to the shadow data to use. 
	@param	clientCamera	reference to the client camera to use. 
	@param	sceneCameras	reference to the scene cameras to use. */
	Direct_Technique(Engine& engine, ShadowData& shadowData, Camera& clientCamera, std::vector<Camera*>& sceneCameras) noexcept;


	// Public Interface Implementations
	virtual void clearCache(const float& deltaTime) noexcept override final;
	virtual void updateCache(const float& deltaTime, ecsWorld& world) noexcept override final;
	virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept override final;


private:
	// Private Methods
	/** Attempt to register the geometry of all light models. */
	void registerLightShapes() noexcept;


	// Private Attributes
	Engine& m_engine;
	Shared_Shader m_shader_Lighting, m_shader_Stencil;
	Shared_Mesh m_shapeCube, m_shapeSphere, m_shapeHemisphere;
	std::pair<GLuint, GLuint> m_cubeParams, m_sphereParams, m_hemisphereParams;
	bool m_geometryReady = false;
	GLuint m_vaoID = 0, m_vboID = 0;
	struct DrawData {
		DynamicBuffer<> bufferCamIndex, visLights, indirectShape;
	};
	int m_drawIndex = 0;
	std::vector<DrawData> m_drawData;
	ecsSystemList m_auxilliarySystems;

	// Shared Attributes
	Direct_Light_Data m_frameData;
	std::vector<Camera*>& m_sceneCameras;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // DIRECT_TECHNIQUE_H