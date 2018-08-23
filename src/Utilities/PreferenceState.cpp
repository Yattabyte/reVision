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

float PreferenceState::getPreference(const Preference & targetKey) const 
{
	if (m_preferences)
		return m_preferences->getValue(targetKey);
	return UNDEFINED_CVAL;
}

void PreferenceState::setPreference(const Preference & targetKey, const float & targetValue) 
{
	if (m_preferences) {
		m_preferences->setValue(targetKey, targetValue);
		if (m_callbacks.find(targetKey) != m_callbacks.end())
			for each (const auto &observer in m_callbacks[targetKey])
				observer.second(targetValue);
	}
}
