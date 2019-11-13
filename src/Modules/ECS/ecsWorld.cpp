#include "Modules/ECS/ecsWorld.h"
#include "Modules/ECS/ecsComponent.h"
#include "Modules/ECS/ECS_M.h"
#include "Modules/ECS/component_types.h"
#include "Engine.h"
#include <random>
#include <sstream>


ecsWorld::~ecsWorld()
{
	clear();
}

ecsWorld::ecsWorld(const std::vector<char>& data)
{
	if (data.size()) {
		size_t dataRead(0ull);
		while (dataRead < data.size())
			deserializeEntity(data.data(), data.size(), dataRead);
	}
}

ecsWorld::ecsWorld(ecsWorld&& other) noexcept
	: m_components(std::move(other.m_components)), m_entities(std::move(other.m_entities))
{
}


/////////////////////////////
/// PUBLIC MAKE FUNCTIONS ///
/////////////////////////////

EntityHandle ecsWorld::makeEntity(ecsBaseComponent** components, const size_t& numComponents, const std::string& name, const EntityHandle& UUID, const EntityHandle& parentUUID)
{
	const auto finalHandle = UUID == EntityHandle() ? (EntityHandle)(generateUUID()) : UUID;
	auto* newEntity = new ecsEntity();
	auto* root = parentUUID.isValid() ? &(getEntity(parentUUID)->m_children) : &m_entities;
	newEntity->m_name = name;
	newEntity->m_entityIndex = (int)root->size();
	newEntity->m_parent = parentUUID;
	root->insert_or_assign(finalHandle, newEntity);

	for (size_t i = 0; i < numComponents; ++i)
		makeComponent(finalHandle, components[i]);

	return finalHandle;
}

ComponentHandle ecsWorld::makeComponent(const EntityHandle& entityHandle, const ecsBaseComponent* component, const ComponentHandle& UUID)
{
	return makeComponent(entityHandle, component->m_runtimeID, component, UUID);
}

ComponentHandle ecsWorld::makeComponent(const EntityHandle& entityHandle, const ComponentID& componentID, const ecsBaseComponent* component, const ComponentHandle& UUID)
{
	auto finalHandle = ComponentHandle();
	// Check if entity is valid
	if (auto* entity = getEntity(entityHandle)) {
		// Check if component ID is valid
		if (isComponentIDValid(componentID)) {
			// Prevent adding duplicate component types to the same entity
			for (const auto& [ID, fn, compHandle] : entity->m_components)
				if (ID == componentID)
					return finalHandle;

			finalHandle = UUID == ComponentHandle() ? (ComponentHandle)(generateUUID()) : UUID;
			const auto& createfn = std::get<0>(ecsBaseComponent::_componentRegistry[componentID]);
			entity->m_components.push_back(std::make_tuple(componentID, createfn(m_components[componentID], finalHandle, entityHandle, component), finalHandle));
		}
	}
	return finalHandle;
}


///////////////////////////////
/// PUBLIC REMOVE FUNCTIONS ///
///////////////////////////////

bool ecsWorld::removeEntity(const EntityHandle& entityHandle)
{
	// Delete this entity's components
	if (auto* entity = getEntity(entityHandle)) {
		for (auto& [id, createFn, componentHandle] : entity->m_components)
			deleteComponent(id, createFn);

		// Delete children entities
		for (const auto& childHandle : getEntityHandles(entityHandle))
			removeEntity(childHandle);

		// Delete this entity
		auto* root = entity->m_parent.isValid() ? &(getEntity(entity->m_parent)->m_children) : &m_entities;
		root->erase(entityHandle);
		delete entity;
		return true;
	}
	return false;
}

bool ecsWorld::removeComponent(const ComponentHandle& componentHandle)
{
	// TO DO
	if (auto* component = getComponent(componentHandle)) {
		return true;
	}
	return false;
}

bool ecsWorld::removeEntityComponent(const EntityHandle& entityHandle, const ComponentID& componentID)
{
	// Check if entity is valid
	if (auto* entity = getEntity(entityHandle)) {
		auto& entityComponents = entity->m_components;
		for (size_t i = 0ull; i < entityComponents.size(); ++i) {
			const auto& [compId, fn, compHandle] = entityComponents[i];
			if (componentID == compId) {
				deleteComponent(compId, fn);
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


////////////////////////////
/// PUBLIC GET FUNCTIONS ///
////////////////////////////

ecsEntity* ecsWorld::getEntity(const EntityHandle& UUID) const
{
	std::function<ecsEntity * (const EntityHandle&, const std::map<EntityHandle, ecsEntity*>&)> find_entity = [&](const EntityHandle& UUID, const std::map<EntityHandle, ecsEntity*>& entities) -> ecsEntity* {
		// First try to find in main map using built-in algorithm
		const auto pos = entities.find(UUID);
		if (pos != entities.end())
			return pos->second;
		// Next start searching children
		else
			for (auto& [handle, entity] : entities)
				if (auto* childEntity = find_entity(UUID, entity->m_children))
					return childEntity;
		return nullptr;
	};
	return find_entity(UUID, m_entities);
}

std::vector<ecsEntity*> ecsWorld::getEntities(const std::vector<EntityHandle>& uuids) const
{
	std::vector<ecsEntity*> entities;
	entities.reserve(uuids.size());
	for each (const auto & uuid in uuids)
		if (auto* entity = getEntity(uuid))
			entities.push_back(entity);
	return entities;
}

std::vector<EntityHandle> ecsWorld::getEntityHandles(const EntityHandle& rootHandle) const
{
	std::vector<EntityHandle> entityHandles;
	auto* root = rootHandle != EntityHandle() ? &getEntity(rootHandle)->m_children : &m_entities;
	entityHandles.reserve(root->size());
	for (const auto& [handle, entity] : *root)
		entityHandles.push_back(handle);
	return entityHandles;
}

ecsBaseComponent* ecsWorld::getComponent(const ComponentHandle& componentHandle) const
{
	std::function<ecsBaseComponent * (const EntityMap&, const ComponentHandle&)> find_component = [&](const EntityMap& entities, const ComponentHandle& componentHandle) -> ecsBaseComponent* {
		// Search all entities in the list supplied
		for (const auto& [entityHandle, entity] : entities) {
			// Check if this entity contains the component handle
			for (const auto& [compID, fn, compHandle] : entity->m_components)
				if (compHandle == componentHandle)
					return (ecsBaseComponent*) & (m_components.at(compID)[fn]);
			// Check if this entity's children contain the component handle
			if (auto* component = find_component(entity->m_children, componentHandle))
				return component;
		}
		return nullptr;
	};

	return find_component(m_entities, componentHandle);
}

ecsBaseComponent* ecsWorld::getComponent(const std::vector<std::tuple<ComponentID, int, ComponentHandle>>& entityComponents, const ComponentDataSpace& mem_array, const ComponentID& componentID) const
{
	for (size_t i = 0ull; i < entityComponents.size(); ++i) {
		const auto& [compId, fn, compHandle] = entityComponents[i];
		if (componentID == compId)
			return (ecsBaseComponent*)&mem_array[fn];
	}
	return nullptr;
}


////////////////////////
/// PUBLIC FUNCTIONS ///
////////////////////////

ecsWorld& ecsWorld::operator=(ecsWorld&& other) noexcept
{
	if (this != &other) {
		m_components = std::move(other.m_components);
		m_entities = std::move(other.m_entities);
	}
	return *this;
}

void ecsWorld::clear()
{
	// Remove all components
	for (auto it = m_components.begin(); it != m_components.end(); ++it) {
		const auto& [createFn, freeFn, newFn, typeSize] = ecsBaseComponent::_componentRegistry[it->first];
		for (size_t i = 0; i < it->second.size(); i += typeSize)
			freeFn((ecsBaseComponent*)&it->second[i]);
		it->second.clear();
	}
	m_components.clear();

	// Remove all entities
	std::function<void(ecsEntity*)> deleteEntity = [&](ecsEntity* entity) {
		for (auto& [handle, child] : entity->m_children)
			deleteEntity(child);
		delete entity;
	};
	for (const auto& [handle, entity] : m_entities)
		deleteEntity(entity);
	m_entities.clear();
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

void ecsWorld::parentEntity(const EntityHandle& parentHandle, const EntityHandle& childHandle)
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
		if (const auto& transformComponent = getComponent<Transform_Component>(parentHandle))
			newParentTransform = transformComponent->m_worldTransform;
	if (childEntity)
		if (childEntity->m_parent.isValid()) {
			const auto& oldParentHandle = childEntity->m_parent;
			if (auto oldParent = getEntity(oldParentHandle)) {
				root = &oldParent->m_children;
				if (const auto& transformComponent = getComponent<Transform_Component>(oldParentHandle))
					oldParentTransform = transformComponent->m_worldTransform;
			}
		}

	// Transform the child entity, removing it from its previous parent's space
	if (const auto& transform = getComponent<Transform_Component>(childHandle)) {
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
	if (const auto& transform = getComponent<Transform_Component>(childHandle)) {
		// Transform the child entity, moving it into its new parent's space
		const auto newPos = newParentTransform.m_inverseModelMatrix * glm::vec4(transform->m_localTransform.m_position, 1.0F);
		transform->m_localTransform.m_position = glm::vec3(newPos) / newPos.w;
		transform->m_localTransform.m_scale /= newParentTransform.m_scale;
		transform->m_localTransform.update();
	}
}

void ecsWorld::unparentEntity(const EntityHandle& entityHandle)
{
	// Move entity up tree, making it a child of its old parent's parent
	if (entityHandle.isValid())
		if (auto* entity = getEntity(entityHandle))
			if (entity->m_parent.isValid())
				if (auto* parent = getEntity(entity->m_parent))
					parentEntity(parent->m_parent.isValid() ? parent->m_parent : EntityHandle(), entityHandle);
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

		auto* srcComponent = reinterpret_cast<ecsBaseComponent*>(&mem_array[srcIndex]);
		auto* destComponent = reinterpret_cast<ecsBaseComponent*>(&mem_array[index]);
		freeFn(destComponent);

		if ((size_t)index == srcIndex) {
			mem_array.resize(srcIndex);
			return;
		}
		std::memcpy(destComponent, srcComponent, typeSize);

		// Update references
		auto& srcComponents = getEntity(srcComponent->m_entity)->m_components;
		for (size_t i = 0; i < srcComponents.size(); ++i) {
			auto& [compID, fn, compHandle] = srcComponents[i];
			if (componentID == compID && static_cast<ComponentID>(srcIndex) == fn) {
				fn = index;
				break;
			}
		}
		mem_array.resize(srcIndex);
	}
}

std::vector<char> ecsWorld::serializeEntities(const std::vector<EntityHandle>& entityHandles) const
{
	return serializeEntities(getEntities(entityHandles));
}

std::vector<char> ecsWorld::serializeEntities(const std::vector<ecsEntity*>& entities) const
{
	std::vector<char> data;
	for each (const auto & entity in entities) {
		if (entity) {
			const auto entData = serializeEntity(*entity);
			data.insert(data.end(), entData.begin(), entData.end());
		}
	}
	return data;
}

std::vector<char> ecsWorld::serializeEntity(const EntityHandle& entityHandle) const
{
	if (const auto& entity = getEntity(entityHandle))
		return serializeEntity(*entity);
	return {};
}

std::vector<char> ecsWorld::serializeEntity(const ecsEntity& entity) const
{
	/* ENTITY DATA STRUCTURE {
		name char count
		name chars
		component data count
		entity child count
		component data
		--nested entity children--
	}*/
	size_t dataIndex(0ull);
	const auto& entityName = entity.m_name;
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
	const auto entityChildCount = (unsigned int)entity.m_children.size();
	std::memcpy(&data[dataIndex], &entityChildCount, sizeof(unsigned int));
	// dataIndex += sizeof(unsigned int); // dataIndex unused, so this can be omitted

	// Accumulate entity component data count
	for (const auto& [componentID, createFunc, componentHandle] : entity.m_components) {
		if (const auto& component = getComponent(entity.m_components, m_components.at(componentID), componentID)) {
			const auto componentData = component->to_buffer();
			data.insert(data.end(), componentData.begin(), componentData.end());
			entityDataCount += componentData.size();
		}
	}
	// Fulfill the entity component data count
	std::memcpy(&data[entityDataCountIndex], &entityDataCount, sizeof(size_t));

	// Write child entities
	for (auto& [childHandle, child] : entity.m_children) {
		const auto childData = serializeEntity(childHandle);
		data.insert(data.end(), childData.begin(), childData.end());
	}

	return data;
}

std::pair<EntityHandle, ecsEntity*> ecsWorld::deserializeEntity(const char* data, const size_t& dataSize, size_t& dataRead, const EntityHandle& parentHandle, const EntityHandle& desiredHandle)
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
	std::memcpy(&nameSize, &data[dataRead], sizeof(unsigned int));
	dataRead += sizeof(unsigned int);
	// Read name chars
	std::memcpy(entityNameChars, &data[dataRead], size_t(nameSize) * sizeof(char));
	dataRead += nameSize * sizeof(char);
	// Read entity component data count
	std::memcpy(&componentDataCount, &data[dataRead], sizeof(size_t));
	dataRead += sizeof(size_t);
	// Read entity child count
	std::memcpy(&entityChildCount, &data[dataRead], sizeof(unsigned int));
	dataRead += sizeof(unsigned int);
	// Find all components between the beginning and end of this entity
	std::vector<std::shared_ptr<ecsBaseComponent>> pointers;
	std::vector<ecsBaseComponent*> components;
	const size_t endIndex = dataRead + componentDataCount;
	while (dataRead < endIndex) {
		const auto& ptr = ecsBaseComponent::from_buffer(data, dataRead);
		pointers.push_back(ptr);
		components.push_back(ptr.get());
	}

	// Make the entity
	auto thisEntityHandle = makeEntity(components.data(), components.size(), std::string(entityNameChars, nameSize), desiredHandle, parentHandle);
	auto* thisEntity = getEntity(thisEntityHandle);

	// Make all child entities
	unsigned int childEntitiesRead(0ull);
	while (childEntitiesRead < entityChildCount && dataRead < dataSize) {
		deserializeEntity(data, dataSize, dataRead, thisEntityHandle);
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
	if (const auto& componentID = nameToComponentID(name)) {
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
				components[k].push_back((ecsBaseComponent*)&mem_array[j]);
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
				componentParam[minSizeIndex] = (ecsBaseComponent*)&mem_array[i];
				if (const auto* entity = getEntity(componentParam[minSizeIndex]->m_entity)) {
					const auto& entityComponents = entity->m_components;

					bool isValid = true;
					for (size_t j = 0; j < componentTypes.size(); ++j) {
						const auto& [componentID, componentFlag] = componentTypes[j];
						if (j == minSizeIndex)
							continue;
						componentParam[j] = getComponent(entityComponents, *componentArrays[j], componentID);
						if ((componentParam[j] == nullptr) && ((unsigned int)componentFlag & (unsigned int)ecsBaseSystem::RequirementsFlag::FLAG_OPTIONAL) == 0) {
							isValid = false;
							break;
						}
					}
					if (isValid)
						components.push_back(componentParam);
				}
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
		if (((unsigned int)componentFlag & (unsigned int)ecsBaseSystem::RequirementsFlag::FLAG_OPTIONAL) != 0)
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