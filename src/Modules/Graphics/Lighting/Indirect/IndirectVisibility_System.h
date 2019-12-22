#pragma once
#ifndef INDIRECTVISIBILITY_SYSTEM_H
#define INDIRECTVISIBILITY_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
struct Indirect_Light_Data;

/** An ECS system responsible for populating render lists PER active perspective in a given frame, for all indirect light related entities. */
class IndirectVisibility_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~IndirectVisibility_System() = default;
	/** Construct this system.
	@param	frameData	reference to common data that changes frame-to-frame. */
	explicit IndirectVisibility_System(Indirect_Light_Data& frameData);


	// Public Interface Implementations
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Attributes
	Indirect_Light_Data& m_frameData;
};

#endif // INDIRECTVISIBILITY_SYSTEM_H