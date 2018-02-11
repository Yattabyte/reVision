/*
	Logic

	- Contains logic for updating gamestate every tick
	- This particular instance just updates the game camera
*/



#pragma once
#ifndef SYSTEM_LOGIC
#define SYSTEM_LOGIC
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\System_Interface.h"
#include "Utilities\Transform.h"


class EnginePackage;
class DT_ENGINE_API System_Logic : public System
{
public: 
	~System_Logic();
	System_Logic();
	void Initialize(EnginePackage *enginePackage);

	void Update(const float &deltaTime);
	void Update_Threaded(const float &deltaTime);

private:
	Transform m_transform;
	vec3 m_rotation;
};

#endif // SYSTEM_LOGIC