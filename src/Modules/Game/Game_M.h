#pragma once
#ifndef GAME_MODULE_H
#define GAME_MODULE_H

#include "Modules\Engine_Module.h"
#include "Modules\Game\Common_Definitions.h"
#include "Modules\Game\Systems\Interface.h"
#include "Utilities\ECS\ecsSystem.h"
#include "Utilities\GL\VectorBuffer.h"



/** A module responsible for the game. */
class Game_Module : public Engine_Module {
public:
	// (de)Constructors
	~Game_Module() = default;
	Game_Module() = default;


	// Public Interface Implementation
	virtual void initialize(Engine * engine) override;


	// Public Methods
	/** Increments the game simulation by a single tick. 
	@param		deltaTime		the delta time. */
	void frameTick(const float & deltaTime);


private:
	// Private Attributes
	bool m_readyToStart = false;
	float m_timeAccumulator = 0.0f;
	std::vector<Game_System_Interface*> m_gameplaySystems;
	ECSSystemList m_systemList;
	BaseECSSystem * m_renderingSystem;
	VectorBuffer<GameBuffer> m_boardBuffer;
	std::vector<VB_Element<GameBuffer>*> m_players;
};

#endif // GAME_MODULE_H