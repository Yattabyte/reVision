/*
	Preferences

	- Loads and updates the preference state for a particular instance of the engine
*/



#pragma once
#ifndef SYSTEM_PREFERENCES
#define SYSTEM_PREFERENCES
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\System_Interface.h"
#include <string>

class Engine_Package;
class DT_ENGINE_API System_Preferences : public System
{
public: 
	~System_Preferences();
	System_Preferences(const std::string &filename = "preferences");
	void Initialize(Engine_Package * package);

	void Update(const float &deltaTime);
	void Update_Threaded(const float &deltaTime);

private:
	std::string m_fileName;
};

#endif // SYSTEM_PREFERENCES