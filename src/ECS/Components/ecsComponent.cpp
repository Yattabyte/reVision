#include "ECS\Components\ecsComponent.h"


std::vector<std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t> >* BaseECSComponent::componentTypes;

const unsigned int BaseECSComponent::registerComponentType(ECSComponentCreateFunction createfn, ECSComponentFreeFunction freefn, const size_t & size) 
{
	if (componentTypes == nullptr)
		componentTypes = new std::vector<std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t> >();
	unsigned int componentID = componentTypes->size();
	componentTypes->push_back(std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t>(createfn, freefn, size));
	return componentID;
}