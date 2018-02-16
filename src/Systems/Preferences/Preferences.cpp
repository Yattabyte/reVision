#include "Systems\Preferences\Preferences.h"
#include "Utilities\EnginePackage.h"



System_Preferences::~System_Preferences()
{
	m_enginePackage->m_Preference_State.Save();
}

System_Preferences::System_Preferences(const std::string & filename) : m_fileName(filename)
{	
}

void System_Preferences::Initialize(EnginePackage * enginePackage)
{ 
	if (!m_Initialized) {
		m_enginePackage = enginePackage;
		m_enginePackage->m_Preference_State.LoadFile(m_fileName);
		m_enginePackage->m_Camera.setDimensions(vec2(m_enginePackage->getPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH), m_enginePackage->getPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT)));
		m_enginePackage->m_Camera.setFarPlane(m_enginePackage->getPreference(PREFERENCE_ENUMS::C_DRAW_DISTANCE));
		m_enginePackage->m_Camera.setGamma(m_enginePackage->getPreference(PREFERENCE_ENUMS::C_GAMMA));
		m_enginePackage->m_Camera.update();
		m_Initialized = true;
	}
}

void System_Preferences::Update(const float & deltaTime)
{
}

void System_Preferences::Update_Threaded(const float & deltaTime)
{
}
