#include "Modules/ECS/ecsWorld.h"
#include "Modules/ECS/ecsComponent.h"
#include "Modules/ECS/ECS_M.h"
#include "Modules/ECS/component_types.h"
#include "Engine.h"
#include <random>
#include <sstream>


ecsWorld::~ecsWorld()
{
	for (auto it = m_components.begin(); it != m_components.end(); ++it) {
		const auto& [createFn, freeFn, newFn, typeSize] = ecsBaseComponent::_componentRegistry[it->first];
		for (size_t i = 0; i < it->second.size(); i += typeSize)
			freeFn((ecsBaseComponent*)& it->second[i]);
		it->second.clear();
	}
	m_components.clear();

	std::function<void(ecsEntity*)> deleteEntity = [&](ecsEntity* entity) {
		for (auto& [handle, child] : entity->m_children)
			deleteEntity(child);
		delete entity;
	};
	for (const auto& [handle, entity] : m_entities)
		deleteEntity(entity);
	m_entities.clear();
}

ecsWorld::ecsWorld(const std::vector<char>& data)
{
	if (data.size()) {
		size_t dataRead(0ull);
		while (dataRead < data.size())
			deserializeEntity(data.data(), data.size(), dataRead);
	}
}

ecsHandle ecsWorld::generateUUID()
{
	std::stringstream ss;
	for (auto i = 0; i < 16; i++) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 255);
		const auto rc = dis(gen);
		std::stringstream hexstream;
		hexstream << std::hex << rc;
		auto hex = hexstream.str();
		ss << (hex.length() < 2 ? '0' + hex : hex);
	}
	const auto& string = ss.str();
	ecsHandle handle;
	std::memcpy(handle.uuid, string.c_str(), size_t(sizeof(char) * 32));
	return handle;
}

ecsHandle ecsWorld::makeEntity(ecsBaseComponent** entityComponents, const size_t& numComponents, const std::string& name, const ecsHandle& UUID, const ecsHandle& parent)
{
	const auto finalHandle = UUID == ecsHandle() ? generateUUID() : UUID;
	auto* newEntity = new ecsEntity();
	auto* root = parent.isValid() ? &(getEntity(parent)->m_children) : &m_entities;
	newEntity->m_name = name;
	newEntity->m_entityIndex = (int)root->size();
	newEntity->m_parent = parent;
	root->insert_or_assign(finalHandle, newEntity);

	for (size_t i = 0; i < numComponents; ++i)
		addComponent(finalHandle, entityComponents[i]);	

	return finalHandle;
}

void ecsWorld::removeEntity(const ecsHandle& entityHandle)
{
	// Delete this entity's components
	auto* entity = getEntity(entityHandle);
	for (auto& [id, createFn] : entity->m_components)
		deleteComponent(id, createFn);

	// Delete children entities
	for (const auto& childHandle : getEntityHandles(entityHandle))
		removeEntity(childHandle);

	// Delete this entity
	auto* root = entity->m_parent.isValid() ? &(getEntity(entity->m_parent)->m_children) : &m_entities;
	root->erase(entityHandle);
	delete entity;
}

bool ecsWorld::addComponent(const ecsHandle& entityHandle, const ecsBaseComponent* component)
{
	return addComponent(entityHandle, component->m_ID, component);
}

bool ecsWorld::addComponent(const ecsHandle& entityHandle, const ComponentID& componentID, const ecsBaseComponent* component)
{
	// Check if entity is valid
	if (auto * entity = getEntity(entityHandle)) {
		// Check if component ID is valid
		if (isComponentIDValid(componentID)) {
			// Prevent adding duplicate component types to the same entity
			for (const auto& [ID, fn] : entity->m_components)
				if (ID == componentID)
					return false;

			const auto componentHandle = generateUUID();
			const auto& createfn = std::get<0>(ecsBaseComponent::_componentRegistry[componentID]);
			std::pair<ComponentID, int> newPair;
			newPair.first = componentID;
			newPair.second = createfn(m_components[componentID], componentHandle, entityHandle, component);
			entity->m_components.push_back(newPair);
			return true;
		}
	}
	return false;
}

bool ecsWorld::removeComponent(const ecsHandle& entityHandle, const ComponentID& componentID)
{
	// Check if entity is valid
	if (auto * entity = getEntity(entityHandle)) {
		auto& entityComponents = entity->m_components;
		for (size_t i = 0ull; i < entityComponents.size(); ++i) {
			if (componentID == entityComponents[i].first) {
				deleteComponent(entityComponents[i].first, entityComponents[i].second);
				const auto srcIndex = entityComponents.size() - 1ull;
				const auto destIndex = i;
				entityComponents[destIndex] = entityComponents[srcIndex];
				entityComponents.pop_back();
				return true;
			}
		}
	}
	return false;
}

ecsBaseComponent* ecsWorld::getComponent(std::vector<std::pair<ComponentID, int>>& entityComponents, ComponentDataSpace& mem_array, const ComponentID& componentID) {
	for (size_t i = 0ull; i < entityComponents.size(); ++i)
		if (componentID == entityComponents[i].first)
			return (ecsBaseComponent*)& mem_array[entityComponents[i].second];
	return nullptr;
}

std::vector<ecsEntity*> ecsWorld::getEntities(const std::vector<ecsHandle>& uuids)
{
	std::vector<ecsEntity*> entities;
	entities.reserve(uuids.size());
	for each (const auto & uuid in uuids)
		if (auto * entity = getEntity(uuid))
			entities.push_back(entity);
	return entities;
}

ecsEntity* ecsWorld::getEntity(const ecsHandle& UUID)
{
	std::function<ecsEntity* (const ecsHandle&, const std::map<ecsHandle, ecsEntity*>&)> find_entity = [&](const ecsHandle& UUID, const std::map<ecsHandle, ecsEntity*>& entities) -> ecsEntity * {
		// First try to find in main map using built-in algorithm
		const auto pos = entities.find(UUID);
		if (pos != entities.end())
			return pos->second;
		// Next start searching children
		else
			for (auto& [handle, entity] : entities)
				if (auto * childEntity = find_entity(UUID, entity->m_children))
					return childEntity;
		return nullptr;
	};
	return find_entity(UUID, m_entities);
}

std::vector<ecsHandle> ecsWorld::getEntityHandles(const ecsHandle& rootHandle)
{
	std::vector<ecsHandle> entityHandles;
	auto* root = rootHandle != ecsHandle() ? &getEntity(rootHandle)->m_children : &m_entities;
	entityHandles.reserve(root->size());
	for (const auto& [handle, entity] : *root)
		entityHandles.push_back(handle);
	return entityHandles;
}


void ecsWorld::parentEntity(const ecsHandle& parentHandle, const ecsHandle& childHandle)
{
	// Validate input parameters
	if (!childHandle.isValid() || parentHandle == childHandle)
		return;

	// Variables
	auto* parentEntity = getEntity(parentHandle), * childEntity = getEntity(childHandle);
	auto* root = &m_entities, * newRoot = parentEntity ? &parentEntity->m_children : &m_entities;
	Transform newParentTransform, oldParentTransform;

	// Check for parent transformations
	if (!childEntity)
		return;
	if (parentEntity)
		if (const auto & transformComponent = getComponent<Transform_Component>(parentHandle))
			newParentTransform = transformComponent->m_worldTransform;
	if (childEntity)
		if (childEntity->m_parent.isValid()) {
			const auto& oldParentHandle = childEntity->m_parent;
			if (auto oldParent = getEntity(oldParentHandle)) {
				root = &oldParent->m_children;
				if (const auto & transformComponent = getComponent<Transform_Component>(oldParentHandle))
					oldParentTransform = transformComponent->m_worldTransform;
			}
		}

	// Transform the child entity, removing it from its previous parent's space
	if (const auto & transform = getComponent<Transform_Component>(childHandle)) {
		const auto newPos = oldParentTransform.m_modelMatrix * glm::vec4(transform->m_localTransform.m_position, 1.0F);
		transform->m_localTransform.m_position = glm::vec3(newPos) / newPos.w;
		transform->m_localTransform.m_scale *= oldParentTransform.m_scale;
		transform->m_localTransform.update();
	}

	// Remove child entity from its previous parent
	root->erase(childHandle);

	// Make this child a child of the new parent, change its index
	childEntity->m_parent = parentHandle;
	childEntity->m_entityIndex = (int)newRoot->size();
	newRoot->insert_or_assign(childHandle, childEntity);
	if (const auto & transform = getComponent<Transform_Component>(childHandle)) {
		// Transform the child entity, moving it into its new parent's space
		const auto newPos = newParentTransform.m_inverseModelMatrix * glm::vec4(transform->m_localTransform.m_position, 1.0F);
		transform->m_localTransform.m_position = glm::vec3(newPos) / newPos.w;
		transform->m_localTransform.m_scale /= newParentTransform.m_scale;
		transform->m_localTransform.update();
	}
}

void ecsWorld::unparentEntity(const ecsHandle& entityHandle)
{
	// Move entity up tree, making it a child of its old parent's parent
	if (entityHandle.isValid())
		if (auto * entity = getEntity(entityHandle))
			if (entity->m_parent.isValid())
				if (auto * parent = getEntity(entity->m_parent))
					parentEntity(parent->m_parent.isValid() ? parent->m_parent : ecsHandle(), entityHandle);
}

bool ecsWorld::isComponentIDValid(const ComponentID& componentID)
{
	return (componentID < ecsBaseComponent::_componentRegistry.size());
}

void ecsWorld::deleteComponent(const ComponentID& componentID, const ComponentID& index)
{
	if (isComponentIDValid(componentID)) {
		auto& mem_array = m_components[componentID];
		const auto& [createFn, freeFn, newFn, typeSize] = ecsBaseComponent::_componentRegistry[componentID];
		const auto srcIndex = mem_array.size() - typeSize;

		auto* srcComponent = (ecsBaseComponent*)& mem_array[srcIndex];
		auto* destComponent = (ecsBaseComponent*)& mem_array[index];
		freeFn(destComponent);

		if ((size_t)index == srcIndex) {
			mem_array.resize(srcIndex);
			return;
		}
		std::memcpy(destComponent, srcComponent, typeSize);

		// Update references
		auto& srcComponents = getEntity(srcComponent->m_entity)->m_components;
		for (size_t i = 0; i < srcComponents.size(); ++i) {
			if (componentID == srcComponents[i].first && (ComponentID)srcIndex == srcComponents[i].second) {
				srcComponents[i].second = index;
				break;
			}
		}
		mem_array.resize(srcIndex);
	}
}

std::vector<char> ecsWorld::serializeEntities(const std::vector<ecsHandle>& entityHandles)
{
	std::vector<char> data;
	for each (const auto & entityHandle in entityHandles) {
		const auto entData = serializeEntity(entityHandle);
		data.insert(data.end(), entData.begin(), entData.end());
	}
	return data;
}

std::vector<char> ecsWorld::serializeEntity(const ecsHandle& entityHandle)
{
	/* ENTITY DATA STRUCTURE {
			name char count
			name chars
			component data count
			entity child count
			component data
			--nested entity children--
	} */
	const auto& entity = getEntity(entityHandle);
	size_t dataIndex(0ull);
	const auto& entityName = entity->m_name;
	const auto nameSize = (unsigned int)(entityName.size());
	const auto ENTITY_HEADER_SIZE = (sizeof(unsigned int) + (entityName.size() * sizeof(char))) + sizeof(size_t) + sizeof(unsigned int);
	std::vector<char> data(ENTITY_HEADER_SIZE);

	// Write name char count
	std::memcpy(&data[dataIndex], &nameSize, sizeof(unsigned int));
	dataIndex += sizeof(unsigned int);
	// Write name chars
	std::memcpy(&data[dataIndex], entityName.c_str(), nameSize * sizeof(char));
	dataIndex += nameSize * sizeof(char);
	// Defer writing entity component data count until later
	size_t entityDataCount(0ull), entityDataCountIndex(dataIndex);
	dataIndex += sizeof(size_t);
	// Write entity child count
	const auto entityChildCount = (unsigned int)entity->m_children.size();
	std::memcpy(&data[dataIndex], &entityChildCount, sizeof(unsigned int));
	dataIndex += sizeof(unsigned int);
	// Accumulate entity component data count
	for (const auto& [componentID, createFunc] : entity->m_components) {
		if (const auto & component = getComponent(entityHandle, componentID)) {
			const auto componentData = component->to_buffer();
			data.insert(data.end(), componentData.begin(), componentData.end());
			entityDataCount += componentData.size();
		}
	}
	// Fulfill the entity component data count
	std::memcpy(&data[entityDataCountIndex], &entityDataCount, sizeof(size_t));

	// Write child entities
	for (auto& [childHandle, child] : entity->m_children) {
		const auto childData = serializeEntity(childHandle);
		data.insert(data.end(), childData.begin(), childData.end());
	}

	return data;
}

std::pair<ecsHandle, ecsEntity*> ecsWorld::deserializeEntity(const char* data, const size_t& dataSize, size_t& dataIndex, const ecsHandle& parentHandle, const ecsHandle& desiredHandle)
{
	/* ENTITY DATA STRUCTURE {
		name char count
		name chars
		component data count
		component data
		entity child count
		--nested entity children--
	} */
	char entityNameChars[256] = { '\0' };
	unsigned int nameSize(0u), entityChildCount(0u);
	size_t componentDataCount(0ull);

	// Read name char count
	std::memcpy(&nameSize, &data[dataIndex], sizeof(unsigned int));
	dataIndex += sizeof(unsigned int);
	// Read name chars
	std::memcpy(entityNameChars, &data[dataIndex], size_t(nameSize) * sizeof(char));
	dataIndex += nameSize * sizeof(char);
	// Read entity component data count
	std::memcpy(&componentDataCount, &data[dataIndex], sizeof(size_t));
	dataIndex += sizeof(size_t);
	// Read enitity child count
	std::memcpy(&entityChildCount, &data[dataIndex], sizeof(unsigned int));
	dataIndex += sizeof(unsigned int);
	// Find all components between the beginning and end of this entity
	std::vector<std::shared_ptr<ecsBaseComponent>> pointers;
	std::vector<ecsBaseComponent*> components;
	const size_t endIndex = dataIndex + componentDataCount;
	while (dataIndex < endIndex) {
		const auto& ptr = ecsBaseComponent::from_buffer(data, dataIndex);
		pointers.push_back(ptr);
		components.push_back(ptr.get());
	}

	// Make the entity
	auto& thisEntityHandle = makeEntity(components.data(), components.size(), std::string(entityNameChars, nameSize), desiredHandle, parentHandle);
	auto* thisEntity = getEntity(thisEntityHandle);

	// Make all child entities
	unsigned int childEntitiesRead(0ull);
	while (childEntitiesRead < entityChildCount && dataIndex < dataSize) {
		deserializeEntity(data, dataSize, dataIndex, thisEntityHandle);
		childEntitiesRead++;
	}
	return { thisEntityHandle, thisEntity };
}

std::optional<ComponentID> ecsWorld::nameToComponentID(const char* name)
{
	return ecsBaseComponent::_nameRegistry.search(name);
}

std::shared_ptr<ecsBaseComponent> ecsWorld::makeComponentType(const char* name)
{
	if (const auto & componentID = nameToComponentID(name)) {
		const auto& [createFn, freeFn, newFn, componentSize] = ecsBaseComponent::_componentRegistry[*componentID];
		return newFn();
	}
	return {};
}

void ecsWorld::updateSystems(ecsSystemList& systems, const float& deltaTime)
{
	for (size_t i = 0; i < systems.size(); ++i)
		updateSystem(systems[i].get(), deltaTime);
}

void ecsWorld::updateSystem(ecsBaseSystem* system, const float& deltaTime)
{
	if (auto components = getRelevantComponents(system->getComponentTypes()); components.size() > 0ull)
		system->updateComponents(deltaTime, components);
}

void ecsWorld::updateSystem(const std::shared_ptr<ecsBaseSystem>& system, const float& deltaTime)
{
	updateSystem(system.get(), deltaTime);
}

void ecsWorld::updateSystem(const float& deltaTime, const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes, const std::function<void(const float&, const std::vector<std::vector<ecsBaseComponent*>>&)>& func)
{
	if (auto components = getRelevantComponents(componentTypes); components.size() > 0ull)
		func(deltaTime, components);
}

std::vector<std::vector<ecsBaseComponent*>> ecsWorld::getRelevantComponents(const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes)
{
	std::vector<std::vector<ecsBaseComponent*>> components;
	if (componentTypes.size() > 0ull) {
		if (componentTypes.size() == 1u) {
			// Super simple procedure for system with 1 component type
			const auto& [componentID, componentFlag] = componentTypes[0];
			const auto& [createFn, freeFn, newFn, typeSize] = ecsBaseComponent::_componentRegistry[componentID];
			const auto& mem_array = m_components[componentID];
			components.resize(mem_array.size() / typeSize);
			for (size_t j = 0, k = 0; j < mem_array.size(); j += typeSize, ++k)
				components[k].push_back((ecsBaseComponent*)& mem_array[j]);
		}
		else {
			// More complex procedure for system with > 1 component type
			std::vector<ecsBaseComponent*> componentParam(componentTypes.size());
			std::vector<ComponentDataSpace*> componentArrays(componentTypes.size());
			for (size_t i = 0; i < componentTypes.size(); ++i)
				componentArrays[i] = &m_components[std::get<0>(componentTypes[i])];

			const auto minSizeIndex = findLeastCommonComponent(componentTypes);
			const auto minComponentID = std::get<0>(componentTypes[minSizeIndex]);
			const auto& [createFn, freeFn, newFn, typeSize] = ecsBaseComponent::_componentRegistry[minComponentID];
			const auto& mem_array = *componentArrays[minSizeIndex];
			components.reserve(mem_array.size() / typeSize); // reserve, not resize, as the component at [i] may be invalid
			for (size_t i = 0; i < mem_array.size(); i += typeSize) {
				componentParam[minSizeIndex] = (ecsBaseComponent*)& mem_array[i];
				auto& entityComponents = getEntity(componentParam[minSizeIndex]->m_entity)->m_components;

				bool isValid = true;
				for (size_t j = 0; j < componentTypes.size(); ++j) {
					const auto& [componentID, componentFlag] = componentTypes[j];
					if (j == minSizeIndex)
						continue;
					componentParam[j] = getComponent(entityComponents, *componentArrays[j], componentID);
					if ((componentParam[j] == nullptr) && (componentFlag & ecsBaseSystem::FLAG_OPTIONAL) == 0) {
						isValid = false;
						break;
					}
				}
				if (isValid)
					components.push_back(componentParam);
			}
		}
	}
	return components;
}

size_t ecsWorld::findLeastCommonComponent(const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes)
{
	auto minSize = std::numeric_limits<size_t>::max();
	auto minIndex = std::numeric_limits<size_t>::max();
	for (size_t i = 0; i < componentTypes.size(); ++i) {
		const auto& [componentID, componentFlag] = componentTypes[i];
		if ((componentFlag & ecsBaseSystem::FLAG_OPTIONAL) != 0)
			continue;

		const auto& [createFn, freeFn, newFn, typeSize] = ecsBaseComponent::_componentRegistry[componentID];
		const auto size = m_components[componentID].size() / typeSize;
		if (size <= minSize) {
			minSize = size;
			minIndex = i;
		}
	}
	return minIndex;
}