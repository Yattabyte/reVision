#pragma once
#ifndef INDIRECTSYNC_SYSTEM_H
#define INDIRECTSYNC_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
struct Indirect_Light_Data;

/** An ECS system responsible for synchronizing indirect light components and sending data to the GPU. */
class IndirectSync_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~IndirectSync_System() = default;
	/** Construct this system.
	@param	frameData	reference to common data that changes frame-to-frame. */
	explicit IndirectSync_System(Indirect_Light_Data& frameData) noexcept;


	// Public Interface Implementations
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


private:
	// Private Attributes
	Indirect_Light_Data& m_frameData;
};

#endif // INDIRECTSYNC_SYSTEM_H