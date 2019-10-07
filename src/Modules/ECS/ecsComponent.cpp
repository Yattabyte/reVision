#include "Modules/ECS/ecsComponent.h"
#include "ecsComponent.h"


std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, ComponentNewFunction, size_t>> ecsBaseComponent::_componentRegistry = std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, ComponentNewFunction, size_t>>();
MappedChar<ComponentID> ecsBaseComponent::_nameRegistry = MappedChar<ComponentID>();

ComponentID ecsBaseComponent::registerType(const ComponentCreateFunction& createFn, const ComponentFreeFunction& freeFn, const ComponentNewFunction& newFn, const size_t& size, const char* string)
{
	const auto componentID = (ComponentID)_componentRegistry.size();
	_componentRegistry.push_back({ createFn, freeFn, newFn, size });
	_nameRegistry.insertOrAssign(string, componentID);

	return componentID;
}

std::shared_ptr<ecsBaseComponent> ecsBaseComponent::from_buffer(const char* data, size_t& dataRead)
{
	// Read Name
	int charCount(0);
	std::memcpy(&charCount, &data[dataRead], sizeof(int));
	dataRead += sizeof(int);
	char* nameString = new char[size_t(charCount) + 1ull];
	std::fill(&nameString[0], &nameString[charCount + 1], '\0');
	std::memcpy(nameString, &data[dataRead], charCount);
	dataRead += sizeof(char) * charCount;
	const auto componentTypeName = std::string(nameString);
	delete[] nameString;
	size_t classDataSize(0ull);
	std::memcpy(&classDataSize, &data[dataRead], sizeof(size_t));
	dataRead += sizeof(size_t);

	// Create new component of class matching the name
	if (const auto& componentID = _nameRegistry.search(componentTypeName.c_str())) [[likely]] {
		const auto & [createFn, freeFn, newFn, size] = _componentRegistry[*componentID];
		const auto& clone = newFn();
		clone->recover_data(&data[dataRead]);
		dataRead += classDataSize;
		return clone;
	}
	dataRead += classDataSize;
	return nullptr;
}