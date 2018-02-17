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

class EnginePackage;


/**
 * An engine system that loads and updates preference state.
 **/
class DT_ENGINE_API System_Preferences : public System
{
public:
	// (de)Constructors
	/** Destroy the preference system. */
	~System_Preferences();
	/** Construct the preference system. 
	 * @param	filename	an optional relative path to the preference file to load. Defaults to "preferences.cfg" */
	System_Preferences(const std::string & filename = "preferences");


	// Interface Implementations
	virtual void initialize(EnginePackage * package);
	virtual void update(const float & deltaTime) {}
	virtual void updateThreaded(const float & deltaTime) {}


private:
	// Private Attributes
	std::string m_fileName;
};

#endif // SYSTEM_PREFERENCES