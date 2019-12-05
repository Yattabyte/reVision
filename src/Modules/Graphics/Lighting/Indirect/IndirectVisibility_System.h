#pragma once
#ifndef INDIRECTVISIBILITY_SYSTEM_H
#define INDIRECTVISIBILITY_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Indirect/IndirectData.h"


/** An ECS system responsible for populating render lists PER active perspective in a given frame, for all indirect light related entities. */
class IndirectVisibility_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~IndirectVisibility_System() = default;
	/** Construct this system.
	@param	frameData	reference to common data that changes frame-to-frame. */
	explicit IndirectVisibility_System(Indirect_Light_Data& frameData) noexcept;


	// Public Interface Implementations
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


private:
	// Private Attributes
	Indirect_Light_Data& m_frameData;
};

#endif // INDIRECTVISIBILITY_SYSTEM_H