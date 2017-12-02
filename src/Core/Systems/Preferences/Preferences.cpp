#include "Systems\Preferences\Preferences.h"
#include "Utilities\Engine_Package.h"



System_Preferences::~System_Preferences()
{
}

System_Preferences::System_Preferences(Engine_Package * package, const std::string & filename) : m_enginePackage(package)
{
	package->m_Preference_State.LoadFile(filename);
	package->m_Camera.setDimensions(vec2(package->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH), package->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT)));
	package->m_Camera.Update();
}

void System_Preferences::Update(const float & deltaTime)
{
}

void System_Preferences::Update_Threaded(const float & deltaTime)
{
}
