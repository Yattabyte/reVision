#include "Modules/World/ECS/ecsComponent.h"


std::vector<std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t>> * BaseECSComponent::componentTypes;
std::map<const char *, int, BaseECSComponent::compare_string> * BaseECSComponent::componentIDMap;

const int BaseECSComponent::registerComponentType(ECSComponentCreateFunction createfn, ECSComponentFreeFunction freefn, const size_t & size, const char * string)
{
	if (componentTypes == nullptr)
		componentTypes = new std::vector<std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t>>();
	const int componentID = (int)componentTypes->size();
	componentTypes->push_back(std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t>(createfn, freefn, size));

	if (componentIDMap == nullptr)
		componentIDMap = new std::map<const char *, int, BaseECSComponent::compare_string>();
	componentIDMap->insert_or_assign(string, componentID);
	return componentID;
}