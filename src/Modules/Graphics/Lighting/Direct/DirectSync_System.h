#pragma once
#ifndef DIRECTSYNC_SYSTEM_H
#define DIRECTSYNC_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Direct/DirectData.h"
#include <algorithm>


/** An ECS system responsible for synchronizing light components and sending data to the GPU. */
class DirectSync_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~DirectSync_System() = default;
	/** Construct this system.
	@param	frameData	reference to common data that changes frame-to-frame. */
	explicit DirectSync_System(Direct_Light_Data& frameData) noexcept;


	// Public Interface Implementations
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


private:
	// Private Attributes
	Direct_Light_Data& m_frameData;
};

#endif // DIRECTSYNC_SYSTEM_H