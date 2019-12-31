#pragma once
#ifndef DIRECTSYNC_SYSTEM_H
#define DIRECTSYNC_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
struct Direct_Light_Data;

/** An ECS system responsible for synchronizing light components and sending data to the GPU. */
class DirectSync_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Construct this system.
	@param	frameData	reference to common data that changes frame-to-frame. */
	explicit DirectSync_System(Direct_Light_Data& frameData);


	// Public Interface Implementations
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Attributes
	Direct_Light_Data& m_frameData;
};

#endif // DIRECTSYNC_SYSTEM_H