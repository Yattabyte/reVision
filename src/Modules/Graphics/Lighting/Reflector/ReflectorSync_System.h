#pragma once
#ifndef REFLECTORSYNC_SYSTEM_H
#define REFLECTORSYNC_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Reflector/ReflectorData.h"


/** An ECS system responsible for synchronizing reflector components and sending data to the GPU. */
class ReflectorSync_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~ReflectorSync_System() = default;
	/** Construct this system.
	@param	frameData	reference to common data that changes frame-to-frame. */
	explicit ReflectorSync_System(ReflectorData& frameData) noexcept;


	// Public Interface Implementations
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


private:
	// Private Attributes
	ReflectorData& m_frameData;
};

#endif // REFLECTORSYNC_SYSTEM_H