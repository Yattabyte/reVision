#include "Systems\Visibility.h"
#include "Utilities\Engine_Package.h"
#include "Rendering\Visibility_Token.h"
#include "Systems\ECS\Components\Geometry_Manager.h"
#include "Systems\ECS\Components\Lighting_Manager.h"

System_Visibility::~System_Visibility()
{
}

System_Visibility::System_Visibility(Engine_Package *package) : m_enginePackage(package)
{

}

void System_Visibility::Update(const float & deltaTime)
{
}

void System_Visibility::Update_Threaded(const float & deltaTime)
{
	Geometry_Manager::CalcVisibility(m_enginePackage->m_Camera);
	Lighting_Manager::CalcVisibility(m_enginePackage->m_Camera);
}
