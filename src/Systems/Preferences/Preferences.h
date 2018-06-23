#pragma once
#ifndef SYSTEM_PREFERENCES_H
#define SYSTEM_PREFERENCES_H

#include "Systems\System_Interface.h"
#include <string>

class Engine;


/**
 * An engine system that loads and updates preference state.
 **/
class System_Preferences : public System
{
public:
	// (de)Constructors
	/** Destroy the preference system. */
	~System_Preferences();
	/** Construct the preference system. 
	 * @param	filename	an optional relative path to the preference file to load. Defaults to "preferences.cfg" */
	System_Preferences(const std::string & filename = "preferences");


	// Interface Implementations
	virtual void initialize(Engine * engine);
	virtual void update(const float & deltaTime) {}
	virtual void updateThreaded(const float & deltaTime) {}


private:
	// Private Attributes
	std::string m_fileName;
};

#endif // SYSTEM_PREFERENCES_H