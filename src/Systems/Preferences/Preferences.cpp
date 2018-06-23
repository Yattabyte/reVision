#include "Systems\Preferences\Preferences.h"
#include "Engine.h"


System_Preferences::~System_Preferences()
{
	m_engine->m_PreferenceState.Save();
}

System_Preferences::System_Preferences(const std::string & filename) : m_fileName(filename)
{	
}

void System_Preferences::initialize(Engine * engine)
{ 
	if (!m_Initialized) {
		m_engine = engine;
		m_engine->m_PreferenceState.LoadFile(m_fileName);
		m_engine->m_Camera->setDimensions(vec2(m_engine->getPreference(PreferenceState::C_WINDOW_WIDTH), m_engine->getPreference(PreferenceState::C_WINDOW_HEIGHT)));
		m_engine->m_Camera->setFarPlane(m_engine->getPreference(PreferenceState::C_DRAW_DISTANCE));
		m_engine->m_Camera->setGamma(m_engine->getPreference(PreferenceState::C_GAMMA));
		m_engine->m_Camera->update();
		m_Initialized = true;
	}
}