#pragma once
#ifndef REFLECTORSCHEDULER_SYSTEM_H
#define REFLECTORSCHEDULER_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include <glad/glad.h>


// Forward Declarations
class Engine;
struct ReflectorData;

/** An ECS system responsible for scheduling when reflector related entities should be updated. */
class ReflectorScheduler_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	~ReflectorScheduler_System();
	/** Construct this system.
	@param	engine		reference to the engine to use. 
	@param	frameData	reference to common data that changes frame-to-frame. */
	ReflectorScheduler_System(Engine& engine, ReflectorData& frameData);


	// Public Interface Implementations
	void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) final;


private:
	// Private Attributes
	Engine& m_engine;
	ReflectorData& m_frameData;
	GLuint m_maxReflectionCasters = 1u;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // REFLECTORSCHEDULER_SYSTEM_H