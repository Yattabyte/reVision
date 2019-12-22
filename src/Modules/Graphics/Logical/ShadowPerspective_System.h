#pragma once
#ifndef SHADOWPERSPECTIVE_SYSTEM_H
#define SHADOWPERSPECTIVE_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
class Camera;

/** An ECS system responsible for collecting cameras from entities with shadow components. */
class ShadowPerspective_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~ShadowPerspective_System() = default;
	/** Construct this system.
	@param	sceneCameras	reference to the scene cameras to use. */
	explicit ShadowPerspective_System(std::vector<Camera*>& sceneCameras);


	// Public Interface Implementations
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Attributes
	std::vector<Camera*>& m_sceneCameras;
};

#endif // SHADOWPERSPECTIVE_SYSTEM_H