#pragma once
#ifndef GAME_MODULE_H
#define GAME_MODULE_H

#include "Modules\Engine_Module.h"
#include "Modules\Game\Components\GameBoard_C.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\ECS\ECS.h"


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
	float m_timeAccumulator = 0.0f;
	ECSSystemList m_gameplaySystems;
	BaseECSSystem * m_renderingSystem;
	VectorBuffer<BoardBuffer> m_boardBuffer;
};

#endif // GAME_MODULE_H