#include "Systems\ECS\Components\Geometry_Manager.h"
#include "Systems\ECS\ComponentFactory.h"
#include "Entities\Components\Geometry_Component.h"


void Geometry_Manager::CalcVisibility(Camera &camera)
{
	shared_lock<shared_mutex> read_guard(camera.getDataMutex());
	Visibility_Token & vis_token = camera.GetVisibilityToken();
	const auto &camBuffer = camera.getCameraBuffer();
	const mat4 camPVMatrix = camBuffer.pMatrix * camBuffer.vMatrix;

	vector<char*> types = { "Anim_Model" };

	for each (auto type in types) {
		const auto components = *((vector<Geometry_Component*>*)(&ComponentFactory::GetComponentsByType(type)));

		vector<Component*> visible_components;

		for each (auto component in components) 
			if (component->IsVisible(camPVMatrix))
				visible_components.push_back((Component*)component);		

		vis_token.insert(pair<char*, vector<Component*>>(type, vector<Component*>()));
		vis_token[type] = visible_components;
	}	
}