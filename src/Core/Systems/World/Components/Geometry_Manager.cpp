#include "Systems\World\Components\Geometry_Manager.h"
#include "Systems\World\Component_Factory.h"
#include "Entities\Components\Geometry_Component.h"



void Geometry_Manager::CalcVisibility(Camera &camera, Component_Factory *componentFactory)
{
	unique_lock<shared_mutex> write_guard(camera.getDataMutex());
	Visibility_Token & vis_token = camera.GetVisibilityToken();
	const auto &camBuffer = camera.getCameraBuffer();
	const mat4 camPVMatrix = camBuffer.pMatrix * camBuffer.vMatrix;

	vector<char*> types = { "Anim_Model" };

	for each (auto type in types) {
		const auto components = *((vector<Geometry_Component*>*)(&componentFactory->GetComponentsByType(type)));

		vector<Component*> visible_components;

		for each (auto component in components) 
			if (component->IsVisible(camPVMatrix))
				visible_components.push_back((Component*)component);		

		vis_token.insert(pair<char*, vector<Component*>>(type, vector<Component*>()));
		vis_token[type] = visible_components;
	}	
}