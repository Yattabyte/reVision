#include "Utilities/PreferenceState.h"


PreferenceState::~PreferenceState() noexcept 
{
	save();
}

PreferenceState::PreferenceState(Engine& engine, const std::string& filename) : 
	m_engine(engine)
{
	loadFile(filename);
}

void PreferenceState::loadFile(const std::string& filename) 
{
	m_preferences = Shared_Config(m_engine, filename, PreferenceState::Preference_Strings(), false);
}

void PreferenceState::save() 
{
	if (m_preferences->ready())
		m_preferences->saveConfig();
}

void PreferenceState::addCallback(const Preference& targetKey, const std::shared_ptr<bool>& alive, const std::function<void(float)>& callback) 
{
	m_callbacks[targetKey].emplace_back(std::pair(alive, callback));
}