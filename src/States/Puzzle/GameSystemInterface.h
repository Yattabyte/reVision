#pragma once
#ifndef GAME_SYSTEM_INTERFACE_H
#define GAME_SYSTEM_INTERFACE_H 

#include "Modules/World/ECS/ecsSystem.h"


/** Interface for game systems */
class Game_System_Interface : public BaseECSSystem {
public:
	// Public (de)Constructors
	/** Default Destructor. */
	inline ~Game_System_Interface() = default;
	/** Default Constructor. */
	inline Game_System_Interface() = default;


	// Public Interface
	/** Returns true if this system is ready to be used, false otherwise. */
	inline virtual bool readyToUse() { return true; }
};

#endif // GAME_SYSTEM_INTERFACE_H