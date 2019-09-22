#include "Modules/ECS/ecsComponent.h"
#include "ecsComponent.h"


std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, size_t>> ecsBaseComponent::_componentRegister = std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, size_t>>();
MappedChar<std::tuple<ecsBaseComponent*, ComponentID, size_t>> ecsBaseComponent::_templateMap = MappedChar<std::tuple<ecsBaseComponent*, ComponentID, size_t>>();

ComponentID ecsBaseComponent::registerType(const ComponentCreateFunction& createFn, const ComponentFreeFunction& freeFn, const size_t& size, const char* string, ecsBaseComponent* templateComponent)
{
	const auto componentID = (ComponentID)_componentRegister.size();
	_componentRegister.push_back({ createFn, freeFn, size });
	_templateMap.insertOrAssign(string, std::make_tuple(templateComponent, componentID, size));

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
	if (const auto & templateParams = _templateMap.search(componentTypeName.c_str())) {
		const auto& [templateComponent, componentID, componentSize] = *templateParams;
		const auto& clone = templateComponent->clone();
		clone->recover_data(&data[dataRead]);
		dataRead += classDataSize;
		return clone;
	}
	dataRead += classDataSize;
	return nullptr;
}