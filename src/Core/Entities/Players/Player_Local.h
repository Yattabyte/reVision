/*
	Player_Local

	- A specialized player, one whom is connected to this machine
*/

#pragma once
#ifndef PLAYER_LOCAL
#define PLAYER_LOCAL
#ifdef	ENGINE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Players\Player.h"

class Player_Local : protected Player
{
public:
	~Player_Local(); 
	Player_Local(); 
};

#endif // PLAYER_LOCAL