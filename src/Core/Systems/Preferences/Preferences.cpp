#include "Systems\Preferences\Preferences.h"
#include "Utilities\Engine_Package.h"



System_Preferences::~System_Preferences()
{
}

System_Preferences::System_Preferences(Engine_Package * package, const std::string & filename) : m_enginePackage(package)
{
	m_enginePackage->m_Preference_State.LoadFile(filename);
	m_enginePackage->m_Camera.setDimensions(vec2(m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH), m_enginePackage->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT)));
	m_enginePackage->m_Camera.Update();
}

void System_Preferences::Shutdown()
{
	m_enginePackage->m_Preference_State.Save();
}

void System_Preferences::Update(const float & deltaTime)
{
}

void System_Preferences::Update_Threaded(const float & deltaTime)
{
}
