#pragma once
#ifndef PLAYERFREELOOK_SYSTEM_H
#define PLAYERFREELOOK_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"


// Forward Declarations
class Engine;

/** A system responsible for updating player components based on keyboard/mouse. */
class PlayerFreeLook_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this free-look system. */
	inline ~PlayerFreeLook_System() noexcept = default;
	/** Construct a free-look system.
	@param	engine		reference to the engine to use. */
	explicit PlayerFreeLook_System(Engine& engine);


	// Public Interface Implementation
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Attributes
	Engine& m_engine;
};

#endif // PLAYERFREELOOK_SYSTEM_H