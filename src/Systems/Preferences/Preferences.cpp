#include "Systems\Preferences\Preferences.h"
#include "Engine.h"


System_Preferences::~System_Preferences()
{
	m_engine->getPreferenceState().Save();
}

System_Preferences::System_Preferences(const std::string & filename) : m_fileName(filename)
{	
}

void System_Preferences::initialize(Engine * engine)
{ 
	if (!m_Initialized) {
		m_engine = engine;
		m_engine->getPreferenceState().LoadFile(m_fileName);
		m_engine->getCamera()->setDimensions(vec2(m_engine->getPreference(PreferenceState::C_WINDOW_WIDTH), m_engine->getPreference(PreferenceState::C_WINDOW_HEIGHT)));
		m_engine->getCamera()->setFarPlane(m_engine->getPreference(PreferenceState::C_DRAW_DISTANCE));
		m_engine->getCamera()->setGamma(m_engine->getPreference(PreferenceState::C_GAMMA));
		m_engine->getCamera()->update();
		m_Initialized = true;
	}
}