#pragma once
#ifndef PROP_TECHNIQUE_H
#define PROP_TECHNIQUE_H

#include "Modules/Graphics/Geometry/Geometry_Technique.h"
#include "Modules/Graphics/Geometry/Prop/PropData.h"
#include "Modules/Graphics/Geometry/Prop/PropUpload_System.h"
#include "Modules/Graphics/Geometry/Prop/PropVisibility_System.h"
#include "Modules/Graphics/Geometry/Prop/PropSync_System.h"
#include "Modules/ECS/ecsSystem.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Engine.h"


/** A core rendering technique for rendering props from a given viewing perspective. */
class Prop_Technique final : public Geometry_Technique {
public:
	// Public (De)Constructors
	/** Destroy this technique. */
	inline ~Prop_Technique() = default;
	/** Construct a prop rendering technique.
	@param	engine			reference to the engine to use.
	@param	sceneCameras	reference to the scene cameras to use. */
	Prop_Technique(Engine& engine, std::vector<Camera*>& sceneCameras) noexcept;


	// Public Interface Implementations
	virtual void clearCache(const float& deltaTime) noexcept override final;
	virtual void updateCache(const float& deltaTime, ecsWorld& world) noexcept override final;
	virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept override final;
	virtual void cullShadows(const float& deltaTime, const std::vector<std::pair<int, int>>& perspectives) noexcept override final;
	virtual void renderShadows(const float& deltaTime) noexcept override final;


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