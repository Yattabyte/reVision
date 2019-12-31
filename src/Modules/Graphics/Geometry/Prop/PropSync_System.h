#pragma once
#ifndef PROPSYNC_SYSTEM_H
#define PROPSYNC_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
struct PropData;

/** An ECS system responsible for synchronizing prop components and sending data to the GPU. */
class PropSync_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Construct this system.
	@param	frameData	reference to of common data that changes frame-to-frame. */
	explicit PropSync_System(PropData& frameData);


	// Public Interface Implementations
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Attributes
	PropData& m_frameData;
};

#endif // PROPSYNC_SYSTEM_H