#pragma once
#ifndef CAMERAPERSPECTIVE_SYSTEM_H
#define CAMERAPERSPECTIVE_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
class Camera;

/** An ECS system responsible for updating a shared pointer with a list of all active cameras in the scene. */
class CameraPerspective_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Construct this system.
	@param	sceneCameras	reference to the scene cameras to use. */
	explicit CameraPerspective_System(std::vector<Camera*>& sceneCameras);


	// Public Interface Implementations
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Attributes
	std::vector<Camera*>& m_sceneCameras;
};

#endif // CAMERAPERSPECTIVE_SYSTEM_H