#pragma once
#ifndef SHADOW_TECHNIQUE_H
#define SHADOW_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowData.h"


// Forward Declarations
class Engine;
class Camera;

/** A rendering technique responsible for rendering/updating shadow casters in the scene. */
class Shadow_Technique final : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destroy this shadow technique. */
	~Shadow_Technique();
	/** Construct a shadow technique.
	@param	engine			reference to the engine to use.
	@param	sceneCameras	reference to the scene cameras to use. */
	Shadow_Technique(Engine& engine, std::vector<Camera*>& sceneCameras);

	// Public Interface Implementations
	void clearCache(const float& deltaTime) noexcept final;
	void updateCache(const float& deltaTime, ecsWorld& world) final;
	void updatePass(const float& deltaTime) final;


	// Public Methods
	/** Retrieve this technique's shadow data.
	@return			the shadow data for this frame. */
	ShadowData& getShadowData() noexcept;


private:
	// Private Methods
	/** Render all the geometry from each light.
	@param	deltaTime	the amount of time passed last frame. */
	void updateShadows(const float& deltaTime);


	// Private but deleted
	/** Disallow default constructor. */
	inline Shadow_Technique() noexcept = delete;
	/** Disallow move constructor. */
	inline Shadow_Technique(Shadow_Technique&&) noexcept = delete;
	/** Disallow copy constructor. */
	inline Shadow_Technique(const Shadow_Technique&) noexcept = delete;
	/** Disallow move assignment. */
	inline Shadow_Technique& operator =(Shadow_Technique&&) noexcept = delete;
	/** Disallow copy assignment. */
	inline Shadow_Technique& operator =(const Shadow_Technique&) noexcept = delete;


	// Private Attributes
	Engine& m_engine;
	ShadowData m_frameData;
	std::vector<Camera*>& m_sceneCameras;
	ecsSystemList m_auxilliarySystems;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SHADOW_TECHNIQUE_H