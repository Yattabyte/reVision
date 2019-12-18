#pragma once
#ifndef PROPVISIBILITY_SYSTEM_H
#define PROPVISIBILITY_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
struct PropData;

/** An ECS system responsible for populating render lists PER active perspective in a given frame, for all prop related entities. */
class PropVisibility_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~PropVisibility_System() noexcept = default;
	/** Construct this system.
	@param	frameData	reference to of common data that changes frame-to-frame. */
	explicit PropVisibility_System(PropData& frameData) noexcept;


	// Public Interface Implementations
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Attributes
	PropData& m_frameData;
};

#endif // PROPVISIBILITY_SYSTEM_H