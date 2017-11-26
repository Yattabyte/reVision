/*
	Player

	- An object representing a player actor in the game system
*/

#pragma once
#ifndef PLAYER
#define PLAYER
#ifdef	ENGINE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Entity.h"
#include "Utilities\Transform.h"

class Player /*: public Entity*/
{
public:
	~Player() {};
	Player() {};
	void setWorldState(const Transform &state) { m_world_state = state; }
	Transform getWorldState() const { return m_world_state; }

protected:
	Transform m_world_state;
};

#endif // PLAYER