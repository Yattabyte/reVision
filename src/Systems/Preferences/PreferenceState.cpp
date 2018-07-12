#include "Systems/Preferences/PreferenceState.h"
#include "Engine.h"


PreferenceState::~PreferenceState() 
{
}

PreferenceState::PreferenceState(Engine * engine, const std::string & filename) 
{
	m_engine = engine;
	LoadFile(filename);
}

void PreferenceState::LoadFile(const std::string & filename)
{
	m_engine->createAsset(m_preferences, filename, PreferenceState::Preference_Strings(), false);
}

void PreferenceState::Save() 
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
