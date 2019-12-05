#include "Modules/ECS/ecsComponent.h"
#include "ecsComponent.h"


std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, ComponentNewFunction, size_t>> ecsBaseComponent::m_componentRegistry = std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, ComponentNewFunction, size_t>>();
MappedChar<ComponentID> ecsBaseComponent::m_nameRegistry = MappedChar<ComponentID>();

ComponentID ecsBaseComponent::registerType(const ComponentCreateFunction& createFn, const ComponentFreeFunction& freeFn, const ComponentNewFunction& newFn, const size_t& size, const char* string) noexcept
{
	const auto componentID = static_cast<ComponentID>(m_componentRegistry.size());
	m_componentRegistry.emplace_back(createFn, freeFn, newFn, size);
	m_nameRegistry.insertOrAssign(string, componentID);

	return componentID;
}

ecsBaseComponent::ecsBaseComponent(const ComponentID& ID, const size_t& size, const char* name) noexcept
	: m_runtimeID(ID), m_size(size), m_name(name) 
{
}

std::shared_ptr<ecsBaseComponent> ecsBaseComponent::from_buffer(const std::vector<char>& data, size_t& dataRead) noexcept
{
	// Read Name
	int charCount(0);
	std::memcpy(&charCount, &data[dataRead], sizeof(int));
	dataRead += sizeof(int);
	char* nameString = new char[size_t(charCount) + 1ULL];
	std::fill(&nameString[0], &nameString[charCount + 1], '\0');
	std::memcpy(nameString, &data[dataRead], charCount);
	dataRead += sizeof(char) * charCount;
	const auto componentTypeName = std::string(nameString);
	delete[] nameString;
	size_t classDataSize(0ULL);
	std::memcpy(&classDataSize, &data[dataRead], sizeof(size_t));
	dataRead += sizeof(size_t);

	// Create new component of class matching the name
	if (const auto& componentID = m_nameRegistry.search(componentTypeName.c_str())) {
		const auto& [createFn, freeFn, newFn, size] = m_componentRegistry[*componentID];
		const auto& clone = newFn();
		clone->recover_data(std::vector(data.begin() + dataRead, data.begin() + dataRead + classDataSize));
		dataRead += classDataSize;
		return clone;
	}
	dataRead += classDataSize;
	return nullptr;
}