/*
	System_Animation

	- Retrieves objects that implement the animate_interface and controls their animation
*/

#pragma once
#ifndef SYSTEM_ANIMATION
#define SYSTEM_ANIMATION
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\System_Interface.h"


class Engine_Package;
class DT_ENGINE_API System_Animation : public System
{
public:
	~System_Animation();
	System_Animation();
	virtual void Initialize(Engine_Package * enginePackage);
	virtual void Update(const float &deltaTime);
	virtual void Update_Threaded(const float &deltaTime);

private:
};

#endif // SYSTEM_ANIMATION