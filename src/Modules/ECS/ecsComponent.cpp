#include "Modules/ECS/ecsComponent.h"


std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, size_t>> ecsBaseComponent::_componentRegister = std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, size_t>>();
MappedChar<std::tuple<ecsBaseComponent*, ComponentID, size_t>> ecsBaseComponent::_templateMap = MappedChar<std::tuple<ecsBaseComponent*, ComponentID, size_t>>();

ComponentID ecsBaseComponent::registerType(const ComponentCreateFunction& createFn, const ComponentFreeFunction& freeFn, const size_t& size, const char* string, ecsBaseComponent* templateComponent)
{
	const auto componentID = (ComponentID)_componentRegister.size();
	_componentRegister.push_back({ createFn, freeFn, size });
	_templateMap.insertOrAssign(string, std::make_tuple(templateComponent, componentID, size));

	return componentID;
}