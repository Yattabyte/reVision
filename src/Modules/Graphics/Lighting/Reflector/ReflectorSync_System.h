#pragma once
#ifndef REFLECTORSYNC_SYSTEM_H
#define REFLECTORSYNC_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
struct ReflectorData;

/** An ECS system responsible for synchronizing reflector components and sending data to the GPU. */
class ReflectorSync_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~ReflectorSync_System() noexcept = default;
	/** Construct this system.
	@param	frameData	reference to common data that changes frame-to-frame. */
	explicit ReflectorSync_System(ReflectorData& frameData) noexcept;


	// Public Interface Implementations
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Attributes
	ReflectorData& m_frameData;
};

#endif // REFLECTORSYNC_SYSTEM_H