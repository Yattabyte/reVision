#include "Systems\Visibility.h"
#include "Rendering\Camera.h"
#include "Rendering\Visibility_Token.h"
#include "Systems\ECS\Components\Geometry_Manager.h"
#include "Systems\ECS\Components\Lighting_Manager.h"

System_Visibility::~System_Visibility()
{
}

System_Visibility::System_Visibility(Camera * engineCamera) : m_engineCamera(engineCamera)
{

}

void System_Visibility::Update(const float & deltaTime)
{
}

void System_Visibility::Update_Threaded(const float & deltaTime)
{
	Geometry_Manager::CalcVisibility(*m_engineCamera);
	Lighting_Manager::CalcVisibility(*m_engineCamera);
}
