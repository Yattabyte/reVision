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

class Camera;
class DT_ENGINE_API System_Visibility : public System
{
public: 
	~System_Visibility();
	System_Visibility(Camera *engineCamera);

	// Render a frame
	void Update(const float &deltaTime);
	void Update_Threaded(const float &deltaTime);


private:
	Camera *m_engineCamera;
};

#endif // SYSTEM_VISIBILITY