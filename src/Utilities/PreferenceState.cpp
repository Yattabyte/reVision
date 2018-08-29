#include "Utilities/PreferenceState.h"
#include "Engine.h"


PreferenceState::~PreferenceState() 
{
}

PreferenceState::PreferenceState(Engine * engine, const std::string & filename) 
{
	m_engine = engine;
	loadFile(filename);
}

void PreferenceState::loadFile(const std::string & filename)
{
	m_preferences = Asset_Config::Create(m_engine, filename, PreferenceState::Preference_Strings(), false);
}

void PreferenceState::save() 
{
	m_preferences->saveConfig();
}

