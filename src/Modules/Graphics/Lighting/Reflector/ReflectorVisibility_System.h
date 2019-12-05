#pragma once
#ifndef REFLECTORVISIBILITY_SYSTEM_H
#define REFLECTORVISIBILITY_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
struct ReflectorData;

/** An ECS system responsible for populating render lists PER active perspective in a given frame, for all reflector related entities. */
class ReflectorVisibility_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~ReflectorVisibility_System() = default;
	/** Construct this system.
	@param	frameData	reference to common data that changes frame-to-frame. */
	explicit ReflectorVisibility_System(ReflectorData& frameData) noexcept;


	// Public Interface Implementations
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


private:
	// Private Attributes
	ReflectorData& m_frameData;
};

#endif // REFLECTORVISIBILITY_SYSTEM_H