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


class Engine_Package;
class DT_ENGINE_API System_Logic : public System
{
public: 
	~System_Logic();
	System_Logic(Engine_Package * package);

	void Update(const float &deltaTime);
	void Update_Threaded(const float &deltaTime);

private:
	Engine_Package *m_enginePackage;
	Transform m_transform;
	vec3 m_rotation;
};

#endif // SYSTEM_LOGIC