#pragma once
#ifndef INDIRECT_TECHNIQUE_H
#define INDIRECT_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/Indirect/IndirectData.h"
#include "Assets/Shader.h"
#include "Assets/Auto_Model.h"
#include "Utilities/GL/StaticMultiBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/IndirectDraw.h"


// Forward Declarations
class RH_Volume;
struct Viewport;

/** A core lighting technique responsible for indirect-diffuse lighting. */
class Indirect_Technique final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destructor. */
	~Indirect_Technique() noexcept;
	/** Construct an indirect lighting technique.
	@param	engine			reference to the engine to use. 
	@param	shadowData		reference to the shadow data to use. 
	@param	clientCamera	reference to the client camera to use. 
	@param	sceneCameras	reference to the scene cameras to use. */
	Indirect_Technique(Engine& engine, ShadowData& shadowData, Camera& clientCamera, std::vector<Camera*>& sceneCameras) noexcept;


	// Public Interface Implementations
	virtual void clearCache(const float& deltaTime) noexcept override final;
	virtual void updateCache(const float& deltaTime, ecsWorld& world) noexcept override final;
	virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept override final;


private:
	// Private Methods
	/** Update the draw parameters for a draw call.	
	@param	camIndicesGen			the camera indexes for GI generation.
	@param	camIndiciesRebounce		the camera indexes for GI re-bounce.
	@param	camIndiciesRecon		the camera indexes for GI reconstruction.
	@param	lightIndices			the light indexes for GI.
	@param	shadowCount				the number of shadows to render for GI.
	@param	perspectivesSize		the number of viewing perspectives. */
	void updateDrawParams(std::vector<glm::ivec2>& camIndicesGen, std::vector<glm::ivec2>& camIndiciesRebounce, std::vector<glm::ivec2>& camIndiciesRecon, std::vector<int>& lightIndices, const size_t& shadowCount, const size_t& perspectivesSize) noexcept;
	/** Populate the radiance hints volume with the first light bounce.
	@param	shadowCount			the number of light casters with shadow maps.
	@param	rhVolume			reference to the RH volume. */
	void fillBounceVolume(const size_t& shadowCount, RH_Volume& rhVolume) noexcept;
	/** Re-bounce the light in the volume a second time.
	@param	rhVolume			reference to the RH volume.
	@param	camBufferRebounce	reference to the buffer for re-bouncing GI.
	@param	indirectQuad		reference to the indirect quad draw call. */
	void rebounceVolume(RH_Volume& rhVolume, DynamicBuffer<>& camBufferRebounce, IndirectDraw<>& indirectQuad) noexcept;
	/** Reconstruct GI from the RH volume. 
	@param	viewport			reference to the active viewport.
	@param	camBufferRecon		reference to the buffer for reconstructing GI.
	@param	indirectQuadRecon	reference to the indirect GI reconstruction draw call. */
	void reconstructVolume(const std::shared_ptr<Viewport>& viewport, DynamicBuffer<>& camBufferRecon, IndirectDraw<>& indirectQuadRecon) noexcept;


	// Private Attributes
	Engine& m_engine;
	Shared_Shader m_shader_Bounce, m_shader_Recon, m_shader_Rebounce;
	Shared_Auto_Model m_shapeQuad;
	GLuint m_textureNoise32 = 0;
	GLuint m_bounceSize = 16u;
	struct DrawData {
		DynamicBuffer<> bufferCamIndex, bufferCamRebounce, bufferCamRecon, visLights;
		StaticMultiBuffer<> indirectBounce = StaticMultiBuffer(sizeof(GLuint) * 4);
		IndirectDraw<> indirectQuad = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT), indirectQuadRecon = IndirectDraw((GLuint)6, 1, 0, GL_DYNAMIC_STORAGE_BIT);
	};
	int m_drawIndex = 0;
	std::vector<DrawData> m_drawData;
	ecsSystemList m_auxilliarySystems;

	// Shared Attributes
	Indirect_Light_Data m_frameData;
	std::vector<Camera*>& m_sceneCameras;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // INDIRECT_TECHNIQUE_H