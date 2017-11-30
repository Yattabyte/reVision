/*
	Visibility

	- Calculates visibility information - whether or not things are visible from a given viewing perspective
*/



#pragma once
#ifndef SYSTEM_VISIBILITY
#define SYSTEM_VISIBILITY
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\System_Interface.h"

class Engine_Package;
class DT_ENGINE_API System_Visibility : public System
{
public: 
	~System_Visibility();
	System_Visibility(Engine_Package *package);

	// Recalculate visibility
	void Update(const float &deltaTime);
	void Update_Threaded(const float &deltaTime);


private:
	Engine_Package *m_enginePackage;
};

#endif // SYSTEM_VISIBILITY