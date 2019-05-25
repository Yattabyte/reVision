#pragma once
#ifndef GAME_SYSTEM_INTERFACE_H
#define GAME_SYSTEM_INTERFACE_H 

#include "Modules/World/ECS/ecsSystem.h"


/** Interface for game systems */
class Game_System_Interface : public BaseECSSystem {
public:
	// (de)Constructors
	~Game_System_Interface() = default;
	Game_System_Interface() = default;


	// Public Interface
	/** Returns true if this system is ready to be used, false otherwise. */
	virtual bool readyToUse() { return true; }
};

#endif // GAME_SYSTEM_INTERFACE_H