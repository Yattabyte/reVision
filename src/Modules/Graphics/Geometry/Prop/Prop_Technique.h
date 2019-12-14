#pragma once
#ifndef PROP_TECHNIQUE_H
#define PROP_TECHNIQUE_H

#include "Modules/Graphics/Geometry/Geometry_Technique.h"
#include "Modules/Graphics/Geometry/Prop/PropData.h"
#include "Modules/Graphics/Common/Camera.h"
#include "Modules/ECS/ecsSystem.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"


/** A core rendering technique for rendering props from a given viewing perspective. */
class Prop_Technique final : public Geometry_Technique {
public:
	// Public (De)Constructors
	/** Destroy this technique. */
	inline ~Prop_Technique() noexcept = default;
	/** Construct a prop rendering technique.
	@param	engine			reference to the engine to use.
	@param	sceneCameras	reference to the scene cameras to use. */
	Prop_Technique(Engine& engine, std::vector<Camera*>& sceneCameras) noexcept;


	// Public Interface Implementations
	void clearCache(const float& deltaTime) noexcept final;
	void updateCache(const float& deltaTime, ecsWorld& world) noexcept final;
	void renderTechnique(const float& deltaTime, Viewport& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept final;
	void cullShadows(const float& deltaTime, const std::vector<std::pair<int, int>>& perspectives) noexcept final;
	void renderShadows(const float& deltaTime) noexcept final;


private:
	// Private Methods
	/** Clear out the props queued up for rendering. */
	void clear() noexcept;


	// Private Attributes
	Engine& m_engine;
	Shared_Shader m_shaderCull, m_shaderGeometry, m_shaderShadowCull, m_shaderShadowGeometry;
	Shared_Auto_Model m_shapeCube;
	struct DrawData {
		DynamicBuffer<> bufferCamIndex, bufferPropIndex, bufferCulling, bufferRender, bufferSkeletonIndex;
	};
	int m_drawIndex = 0;
	size_t m_count = 0ull;
	std::vector<DrawData> m_drawData;
	ecsSystemList m_auxilliarySystems;


	// Shared Attributes
	PropData m_frameData;
	std::vector<Camera*>& m_sceneCameras;
};

#endif // PROP_TECHNIQUE_H