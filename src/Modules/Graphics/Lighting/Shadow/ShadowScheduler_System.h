#pragma once
#ifndef SHADOWSCHEDULER_SYSTEM_H
#define SHADOWSCHEDULER_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowData.h"
#include "Engine.h"


/** An ECS system responsible for scheduling when light & shadow related entities should be updated. */
class ShadowScheduler_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	~ShadowScheduler_System() noexcept;
	/** Construct this system.
	@param	engine		reference to the engine to use. 
	@param	frameData	reference to common data that changes frame-to-frame. */
	ShadowScheduler_System(Engine& engine, ShadowData& frameData) noexcept;


	// Public Interface Implementations
	virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final;


private:
	// Private Attributes
	Engine& m_engine;
	ShadowData& m_frameData;
	GLuint m_maxShadowsCasters = 1u;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SHADOWSCHEDULER_SYSTEM_H