#include "Modules/World/ECS/ecsComponent.h"


std::vector<std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t>> * BaseECSComponent::componentTypes;
std::map<const char *, std::tuple<BaseECSComponent *, int, size_t>, BaseECSComponent::compare_string> * BaseECSComponent::templateMap;

int BaseECSComponent::registerComponentType(ECSComponentCreateFunction createfn, ECSComponentFreeFunction freefn, const size_t & size, const char * string, BaseECSComponent * templateComponent)
{
	if (componentTypes == nullptr)
		componentTypes = new std::vector<std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t>>();
	const int componentID = (int)componentTypes->size();
	componentTypes->push_back(std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t>(createfn, freefn, size));

	if (templateMap == nullptr)
		templateMap = new std::map<const char *, std::tuple<BaseECSComponent *, int, size_t>, BaseECSComponent::compare_string>();
	templateMap->insert_or_assign(string, std::make_tuple(templateComponent, componentID, size));
	return componentID;
}

std::tuple<BaseECSComponent *, int, size_t> BaseECSComponent::findTemplate(const char * name)
{
	if (templateMap->find(name) != templateMap->end())
		return templateMap->at(name);
	return { nullptr, 0, 0ull};
}
