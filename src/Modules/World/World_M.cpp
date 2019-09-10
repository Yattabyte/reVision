#include "Modules/World/World_M.h"
#include "Modules/World/ECS/components.h"
#include "Engine.h"
#include <fstream>
#include <filesystem>
#include <random>
#include <sstream>


void World_Module::initialize(Engine* engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: World...");
}

void World_Module::deinitialize()
{
	// Update indicator
	m_engine->getManager_Messages().statement("Unloading Module: World...");
	*m_aliveIndicator = false;
	unloadWorld();
}

void World_Module::frameTick(const float& deltaTime)
{
	auto& assetManager = m_engine->getManager_Assets();

	// Firstly, check and see if the following systems are ready
	if (!assetManager.readyToUse())
		return;

	// Signal that the map has finished loading ONCE
	if (m_state == startLoading)
		notifyListeners(finishLoading);
	else if (m_state == finishLoading) {
		// Lastly, check and see if we observed any changes
		if (assetManager.hasChanged())
			notifyListeners(updated);
	}
}

void World_Module::loadWorld(const std::string& mapName)
{
	unloadWorld();

	// Signal that a new map is begining to load
	notifyListeners(startLoading);

	// Read ecsData from disk
	const auto path = Engine::Get_Current_Dir() + "\\Maps\\" + mapName;
	std::vector<char> ecsData(std::filesystem::file_size(path));
	std::ifstream mapFile(path, std::ios::binary | std::ios::beg);
	if (!mapFile.is_open())
		m_engine->getManager_Messages().error("Cannot read the binary map file from disk!");
	else
		mapFile.read(ecsData.data(), (std::streamsize)ecsData.size());
	mapFile.close();

	if (ecsData.size()) {
		size_t dataRead(0ull);
		while (dataRead < ecsData.size())
			deserializeEntity(ecsData.data(), ecsData.size(), dataRead, nullptr);
	}
}

void World_Module::saveWorld(const std::string& mapName)
{
	std::vector<char> data;
	for (const auto & [handle, entity] : m_entities) {
		const auto entityData = serializeEntity(entity);
		data.insert(data.end(), entityData.begin(), entityData.end());
	}

	// Write ecsData to disk
	std::fstream mapFile(Engine::Get_Current_Dir() + "\\Maps\\" + mapName, std::ios::binary | std::ios::out);
	if (!mapFile.is_open())
		m_engine->getManager_Messages().error("Cannot write the binary map file to disk!");
	else
		mapFile.write(data.data(), (std::streamsize)data.size());
	mapFile.close();
}

void World_Module::unloadWorld()
{
	for (auto it = m_components.begin(); it != m_components.end(); ++it) {
		auto typeSize = BaseECSComponent::getTypeSize(it->first);
		auto freefn = BaseECSComponent::getTypeFreeFunction(it->first);
		for (size_t i = 0; i < it->second.size(); i += typeSize)
			freefn((BaseECSComponent*)& it->second[i]);
		it->second.clear();
	}
	m_components.clear();

	std::function<void(ecsEntity*)> deleteEntity = [&](ecsEntity* entity) {
		for (auto & [handle, child] : entity->m_children)
			deleteEntity(child);
		delete entity;
	};
	for (const auto& [handle, entity] : m_entities)
		deleteEntity(entity);
	m_entities.clear();

	// Signal that the last map has unloaded
	notifyListeners(unloaded);
}

std::vector<char> World_Module::serializeEntities(const std::vector<ecsEntity*> entities)
{
	std::vector<char> data;
	for each (const auto & entity in entities) {
		const auto entData = serializeEntity(entity);
		data.insert(data.end(), entData.begin(), entData.end());
	}
	return data;
}

std::vector<char> World_Module::serializeEntity(ecsEntity* entity)
{
	/* ENTITY DATA STRUCTURE {
			name char count
			name chars
			component data count
			entity child count
			component data
			--nested entity children--
	} */
	size_t dataIndex(0ull);
	const auto& entityName = entity->m_name;
	const auto nameSize = (unsigned int)(entityName.size());
	const auto ENTITY_HEADER_SIZE = (32 * sizeof(char)) + (sizeof(unsigned int) + (entityName.size() * sizeof(char))) + sizeof(size_t) + sizeof(unsigned int);
	std::vector<char> data(ENTITY_HEADER_SIZE);

	// Write UUID
	std::memcpy(&data[dataIndex], entity->m_uuid.uuid, 32 * sizeof(char));
	dataIndex += 32 * sizeof(char);
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
		const auto componentData = serializeComponent(getComponentInternal(entity->m_components, m_components[componentID], componentID));
		data.insert(data.end(), componentData.begin(), componentData.end());
		entityDataCount += componentData.size();
	}
	// Fulfill the entity component data count
	std::memcpy(&data[entityDataCountIndex], &entityDataCount, sizeof(size_t));

	// Write child entities
	for (auto & [handle, child] : entity->m_children) {
		const auto childData = serializeEntity(child);
		data.insert(data.end(), childData.begin(), childData.end());
	}

	return data;
}

ecsEntity* World_Module::deserializeEntity(const char* data, const size_t& dataSize, size_t& dataIndex, ecsEntity* parent)
{
	/* ENTITY DATA STRUCTURE {
		name char count
		name chars
		component data count
		component data
		entity child count
		--nested entity children--
	} */
	char entityHandle[32] = { '\0' };
	char entityNameChars[256] = { '\0' };
	unsigned int nameSize(0u), entityChildCount(0u);
	size_t componentDataCount(0ull);

	// Read UUID
	std::memcpy(entityHandle, &data[dataIndex], 32 * sizeof(char));
	dataIndex += 32 * sizeof(char);
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
	std::vector<BaseECSComponent*> components;
	std::vector<int> componentIDS;
	const size_t endIndex = dataIndex + componentDataCount;
	while (dataIndex < endIndex) {
		const auto& [component, id] = deserializeComponent(data, dataSize, dataIndex);
		components.push_back(component);
		componentIDS.push_back(id);
	}

	// Make the entity		
	auto& thisentityHandle = makeEntity(components.data(), componentIDS.data(), components.size(), std::string(entityNameChars, nameSize), ecsHandle(entityHandle), parent);
	auto* thisEntity = getEntity(thisentityHandle);
	// Delete temporary components
	for each (auto * component in components)
		delete component;

	// Make all child entities
	unsigned int childEntitiesRead(0ull);
	while (childEntitiesRead < entityChildCount && dataIndex < dataSize) {
		deserializeEntity(data, dataSize, dataIndex, thisEntity);
		childEntitiesRead++;
	}
	return thisEntity;
}

std::vector<char> World_Module::serializeComponent(BaseECSComponent* component)
{
	if (component) {
		const auto componentData = component->save();
		const auto componentDataSize = componentData.size();

		std::vector<char> data(sizeof(size_t) + componentDataSize);
		std::memcpy(&data[0], &componentDataSize, sizeof(size_t));
		std::memcpy(&data[sizeof(size_t)], &componentData[0], componentDataSize);

		return data;
	}
	return {};
}

std::pair<BaseECSComponent*, int> World_Module::deserializeComponent(const char* data, const size_t& dataSize, size_t& dataIndex)
{
	// Find how large the component data is
	size_t componentDataSize(0ull);
	std::memcpy(&componentDataSize, &data[dataIndex], sizeof(size_t));
	dataIndex += sizeof(size_t);

	// Retrieve component Data
	std::vector<char> componentData(componentDataSize);
	std::memcpy(&componentData[0], &data[dataIndex], componentDataSize);
	dataIndex += componentDataSize;

	// Read component name from the data
	size_t nameDataRead(0ull);
	int nameCount(0);
	std::memcpy(&nameCount, &componentData[0], sizeof(int));
	nameDataRead += sizeof(int);
	char* chars = new char[size_t(nameCount) + 1ull];
	std::fill(&chars[0], &chars[nameCount + 1], '\0');
	std::memcpy(chars, &componentData[sizeof(int)], nameCount);
	nameDataRead += sizeof(char) * nameCount;
	const auto componentTypeName = std::string(chars);
	delete[] chars;

	std::vector<char> serializedComponentData;
	if (componentDataSize - nameDataRead)
		serializedComponentData = std::vector<char>(componentData.begin() + nameDataRead, componentData.end());

	std::pair<BaseECSComponent*, int> compPair = { nullptr, -1 };
	if (const auto & [templateComponent, componentID, componentSize] = BaseECSComponent::findTemplate(componentTypeName.c_str()); templateComponent != nullptr) {
		// Clone the template component completely, then fill in the serialized data
		auto* castedComponent = templateComponent->clone();
		if (serializedComponentData.size())
			castedComponent->load(serializedComponentData);
		castedComponent->entity = NULL_ENTITY_HANDLE;

		compPair = { castedComponent , componentID };
	}
	return compPair;
}

void World_Module::addLevelListener(const std::shared_ptr<bool>& alive, const std::function<void(const WorldState&)>& func)
{
	m_notifyees.push_back(std::make_pair(alive, func));
}

ecsHandle World_Module::makeEntity(BaseECSComponent** entityComponents, const int* componentIDs, const size_t& numComponents, const std::string& name, const ecsHandle& UUID, ecsEntity* parent)
{
	auto* newEntity = new ecsEntity();
	for (size_t i = 0; i < numComponents; ++i) {
		if (BaseECSComponent::isTypeValid(componentIDs[i]))
			addComponentInternal(newEntity, componentIDs[i], entityComponents[i]);
		else
			m_engine->getManager_Messages().error("ECS Error: attempted to add an unsupported component to the entity \"" + name + "\"! Skipping component...\r\n");
	}

	auto* root = parent ? &parent->m_children : &m_entities;
	newEntity->m_name = name;
	newEntity->m_entityIndex = (int)root->size();
	newEntity->m_uuid = UUID == ecsHandle() ? generateUUID() : UUID;
	newEntity->m_parent = parent;
	root->insert_or_assign(newEntity->m_uuid, newEntity);

	return newEntity->m_uuid;
}

void World_Module::removeEntity(const ecsHandle& entityHandle)
{
	// Delete this entity's components
	auto* entity = getEntity(entityHandle);
	for (auto& [id, createFn] : entity->m_components)
		deleteComponent(id, createFn);
	
	// Delete children entities
	for (auto& [handle, child] : entity->m_children)
		removeEntity(child->m_uuid);

	// Delete this entity
	auto* root = entity->m_parent ? &entity->m_parent->m_children : &m_entities;
	root->erase(entityHandle);
	delete entity;
}

void World_Module::parentEntity(const ecsHandle& parentHandle, const ecsHandle& childHandle)
{
	// Validate input parameters
	if (!childHandle.isValid() || parentHandle == childHandle)
		return;

	// Variables
	auto* parentEntity = getEntity(parentHandle), * childEntity = getEntity(childHandle);
	auto* root = &m_entities, * newRoot = parentEntity ? &parentEntity->m_children : &m_entities;
	Transform newParentTransform, oldParentTransform;

	// Check for parent transformations
	if (parentEntity)
		if (const auto & transformComponent = getComponent<Transform_Component>(parentHandle))
			newParentTransform = transformComponent->m_worldTransform;
	if (childEntity)
		if (auto * oldParent = childEntity->m_parent) {
			root = &oldParent->m_children;
			if (const auto & transformComponent = getComponent<Transform_Component>(oldParent->m_uuid))
				oldParentTransform = transformComponent->m_worldTransform;
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
	childEntity->m_parent = parentEntity;
	childEntity->m_entityIndex = newRoot->size();
	newRoot->insert_or_assign(childHandle, childEntity);
	if (const auto & transform = getComponent<Transform_Component>(childHandle)) {
		// Transform the child entity, moving it into its new parent's space
		const auto newPos = newParentTransform.m_inverseModelMatrix * glm::vec4(transform->m_localTransform.m_position, 1.0F);
		transform->m_localTransform.m_position = glm::vec3(newPos) / newPos.w;
		transform->m_localTransform.m_scale /= newParentTransform.m_scale;
		transform->m_localTransform.update();
	}
}

void World_Module::unparentEntity(const ecsHandle& entityHandle)
{
	// Move entity up tree, making it a child of its old parent's parent
	if (entityHandle.isValid())
		if (auto* entity = getEntity(entityHandle))
			if (auto* parent = entity->m_parent)
				parentEntity(parent->m_parent == nullptr ? ecsHandle() : parent->m_parent->m_uuid, entityHandle);
}

std::vector<ecsHandle> World_Module::getUUIDs(const std::vector<ecsEntity*>& entities)
{
	std::vector<ecsHandle> uuids;
	for each (const auto & entity in entities)
		uuids.push_back(entity->m_uuid);
	return uuids;
}

ecsHandle World_Module::getUUID(const ecsEntity* entity)
{
	return entity->m_uuid;
}

std::vector<ecsEntity*> World_Module::getEntities(const std::vector<ecsHandle>& uuids)
{
	std::vector<ecsEntity*> entities;
	entities.reserve(uuids.size());
	for each (const auto & uuid in uuids)
		if (auto * entity = getEntity(uuid))
			entities.push_back(entity);
	return entities;
}

ecsEntity* World_Module::getEntity(const ecsHandle& UUID)
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

std::vector<ecsHandle> World_Module::getEntityHandles(const ecsHandle& rootHandle)
{
	std::vector<ecsHandle> entityHandles;
	auto* root = rootHandle != ecsHandle() ? &getEntity(rootHandle)->m_children : &m_entities;
	entityHandles.reserve(root->size());
	for (const auto& [handle, entity] : *root)
		entityHandles.push_back(handle);
	return entityHandles;
}

void World_Module::updateSystems(ECSSystemList& systems, const float& deltaTime)
{
	for (size_t i = 0; i < systems.size(); ++i)
		updateSystem(systems[i].get(), deltaTime);
}

void World_Module::updateSystem(BaseECSSystem* system, const float& deltaTime)
{
	if (auto components = getRelevantComponents(system->getComponentTypes(), system->getComponentFlags()); components.size() > 0ull)
		system->updateComponents(deltaTime, components);
}

void World_Module::updateSystem(const float& deltaTime, const std::vector<int>& types, const std::vector<int>& flags, const std::function<void(const float&, const std::vector<std::vector<BaseECSComponent*>>&)>& func)
{
	if (auto components = getRelevantComponents(types, flags); components.size() > 0ull)
		func(deltaTime, components);
}

void World_Module::notifyListeners(const WorldState& state)
{
	// Get all callback functions, and call them if their owners are still alive
	for (int x = 0; x < (int)m_notifyees.size(); ++x) {
		const auto& [alive, func] = m_notifyees[x];
		if (alive && *(alive.get()) == true)
			func(state);
		else {
			m_notifyees.erase(m_notifyees.begin() + x);
			x--;
		}
	}
	m_state = state;
}

void World_Module::deleteComponent(const int& componentID, const int& index)
{
	auto& mem_array = m_components[componentID];
	auto freefn = BaseECSComponent::getTypeFreeFunction(componentID);
	const auto typeSize = BaseECSComponent::getTypeSize(componentID);
	const auto srcIndex = mem_array.size() - typeSize;

	auto* srcComponent = (BaseECSComponent*)& mem_array[srcIndex];
	auto* destComponent = (BaseECSComponent*)& mem_array[index];
	freefn(destComponent);

	if ((size_t)index == srcIndex) {
		mem_array.resize(srcIndex);
		return;
	}
	std::memcpy(destComponent, srcComponent, typeSize);

	// Update references
	auto& srcComponents = srcComponent->entity->m_components;
	for (size_t i = 0; i < srcComponents.size(); ++i) {
		if (componentID == srcComponents[i].first && (int)srcIndex == srcComponents[i].second) {
			srcComponents[i].second = index;
			break;
		}
	}
	mem_array.resize(srcIndex);
}

bool World_Module::addComponentInternal(ecsEntity* entity, const int& componentID, const BaseECSComponent* component)
{
	// Prevent adding duplicate component types to the same entity
	for (const auto& [ID, fn] : entity->m_components)
		if (ID == componentID)
			return false;

	auto createfn = BaseECSComponent::getTypeCreateFunction(componentID);
	std::pair<int, int> newPair;
	newPair.first = componentID;
	newPair.second = createfn(m_components[componentID], entity, component);
	entity->m_components.push_back(newPair);
	return true;
}

bool World_Module::removeComponentInternal(ecsEntity* entity, const int& componentID)
{
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
	return false;
}

BaseECSComponent* World_Module::getComponentInternal(std::vector<std::pair<int, int>>& entityComponents, std::vector<uint8_t>& mem_array, const int& componentID)
{
	for (size_t i = 0ull; i < entityComponents.size(); ++i)
		if (componentID == entityComponents[i].first)
			return (BaseECSComponent*)& mem_array[entityComponents[i].second];
	return nullptr;
}

std::vector<std::vector<BaseECSComponent*>> World_Module::getRelevantComponents(const std::vector<int>& componentTypes, const std::vector<int>& componentFlags)
{
	std::vector<std::vector<BaseECSComponent*>> components;
	if (componentTypes.size() > 0ull) {
		if (componentTypes.size() == 1u) {
			// Super simple procedure for system with 1 component type
			const auto typeSize = BaseECSComponent::getTypeSize(componentTypes[0]);
			const auto& mem_array = m_components[componentTypes[0]];
			components.resize(mem_array.size() / typeSize);
			for (size_t j = 0, k = 0; j < mem_array.size(); j += typeSize, ++k)
				components[k].push_back((BaseECSComponent*)& mem_array[j]);
		}
		else {
			// More complex procedure for system with > 1 component type
			std::vector<BaseECSComponent*> componentParam(componentTypes.size());
			std::vector<std::vector<uint8_t>*> componentArrays(componentTypes.size());
			for (size_t i = 0; i < componentTypes.size(); ++i)
				componentArrays[i] = &m_components[componentTypes[i]];

			const auto minSizeIndex = findLeastCommonComponent(componentTypes, componentFlags);
			const auto typeSize = BaseECSComponent::getTypeSize(componentTypes[minSizeIndex]);
			const std::vector<uint8_t>& mem_array = *componentArrays[minSizeIndex];
			components.reserve(mem_array.size() / typeSize); // reserve, not resize, as the component at [i] may be invalid
			for (size_t i = 0; i < mem_array.size(); i += typeSize) {
				componentParam[minSizeIndex] = (BaseECSComponent*)& mem_array[i];
				auto& entityComponents = componentParam[minSizeIndex]->entity->m_components;

				bool isValid = true;
				for (size_t j = 0; j < componentTypes.size(); ++j) {
					if (j == minSizeIndex)
						continue;
					componentParam[j] = getComponentInternal(entityComponents, *componentArrays[j], componentTypes[j]);
					if ((componentParam[j] == nullptr) && (componentFlags[j] & BaseECSSystem::FLAG_OPTIONAL) == 0) {
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

size_t World_Module::findLeastCommonComponent(const std::vector<int>& componentTypes, const std::vector<int>& componentFlags)
{
	auto minSize = (size_t)(-1ull);
	auto minIndex = (size_t)(-1ull);
	for (size_t i = 0; i < componentTypes.size(); ++i) {
		if ((componentFlags[i] & BaseECSSystem::FLAG_OPTIONAL) != 0)
			continue;
		const auto typeSize = BaseECSComponent::getTypeSize(componentTypes[i]);
		const auto size = m_components[componentTypes[i]].size() / typeSize;
		if (size <= minSize) {
			minSize = size;
			minIndex = i;
		}
	}
	return minIndex;
}

ecsHandle World_Module::generateUUID()
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