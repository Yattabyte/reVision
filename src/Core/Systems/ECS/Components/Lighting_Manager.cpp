#include "Systems\ECS\Components\Lighting_Manager.h"
#include "Systems\ECS\ComponentFactory.h"
#include "Entities\Components\Lighting_Component.h"



void Lighting_Manager::CalcVisibility(Camera &camera)
{
	unique_lock<shared_mutex> write_guard(camera.getDataMutex());
	Visibility_Token & vis_token = camera.GetVisibilityToken();
	const auto &camBuffer = camera.getCameraBuffer();
	const mat4 camPVMatrix = camBuffer.pMatrix * camBuffer.vMatrix;

	vector<char*> types = { "Light_Directional" };

	for each (auto type in types) {
		const auto components = *((vector<Lighting_Component*>*)(&ComponentFactory::GetComponentsByType(type)));

		vector<Component*> visible_components;

		for each (auto component in components)
			if (component->IsVisible(camPVMatrix))
				visible_components.push_back((Component*)component);

		vis_token.insert(pair<char*, vector<Component*>>(type, vector<Component*>()));
		vis_token[type] = visible_components;
	}
}