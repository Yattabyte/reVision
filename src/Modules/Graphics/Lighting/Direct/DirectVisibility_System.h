#pragma once
#ifndef DIRECTVISIBILITY_SYSTEM_H
#define DIRECTVISIBILITY_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Direct/DirectData.h"


/** An ECS system responsible for populating render lists PER active perspective in a given frame, for all light related entities. */
class DirectVisibility_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~DirectVisibility_System() = default;
	/** Construct this system.
	@param	frameData	reference to common data that changes frame-to-frame. */
	explicit DirectVisibility_System(Direct_Light_Data& frameData) noexcept;


	// Public Interface Implementations
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


private:
	// Private Attributes
	Direct_Light_Data& m_frameData;
};

#endif // DIRECTVISIBILITY_SYSTEM_H