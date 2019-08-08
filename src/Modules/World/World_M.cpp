#include "Modules/World/World_M.h"
#include "Modules/World/ECS/components.h"
#include "Utilities/IO/Level_IO.h"
#include "Engine.h"
#include <fstream>
#include <filesystem>


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

	m_level = Shared_Level(m_engine, mapName);
	m_level->addCallback(m_aliveIndicator, std::bind(&World_Module::processLevel, this));
}

void World_Module::saveWorld(const std::string& mapName)
{
	std::vector<char> ecsData;
	std::function<void(ecsEntity*)> writeOutEntity = [&](ecsEntity* entity) {
		/* ENTITY DATA STRUCTURE {
			name char count
			name chars
			component data count
			entity child count
			component data
			--nested entity children--
		} */
		const auto previousSize = ecsData.size();
		const auto& entityName = entity->m_name;
		const auto nameSize = (unsigned int)(entityName.size());
		const auto ENTITY_HEADER_SIZE = (sizeof(unsigned int) + (entityName.size() * sizeof(char))) + sizeof(size_t) + sizeof(unsigned int);
		ecsData.resize(previousSize + ENTITY_HEADER_SIZE);

		// Write name char count
		size_t dataIndex(previousSize);
		std::memcpy(&ecsData[dataIndex], &nameSize, sizeof(unsigned int)); 
		dataIndex += sizeof(unsigned int);
		// Write name chars
		std::memcpy(&ecsData[dataIndex], entityName.c_str(), nameSize * sizeof(char));
		dataIndex += nameSize * sizeof(char);
		// Defer writing entity component data count until later
		size_t entityDataCount(0ull), entityDataCountIndex(dataIndex);
		dataIndex += sizeof(size_t);
		// Write entity child count
		const auto entityChildCount = (unsigned int)entity->m_children.size();
		std::memcpy(&ecsData[dataIndex], &entityChildCount, sizeof(unsigned int));
		dataIndex += sizeof(unsigned int);				
		// Accumulate entity component data count
		for (const auto& [componentID, createFunc] : entity->m_components) {
			const auto componentData = getComponentInternal(entity->m_components, m_components[componentID], componentID)->save();
			const auto componentDataSize = componentData.size();
			entityDataCount += sizeof(size_t) + componentDataSize;

			// Write component data size + component data to ecs data
			const auto oldDataSize = ecsData.size();
			ecsData.resize(oldDataSize + sizeof(size_t) + componentDataSize);
			std::memcpy(&ecsData[oldDataSize], &componentDataSize, sizeof(size_t));
			std::memcpy(&ecsData[oldDataSize + sizeof(size_t)], &componentData[0], componentDataSize);
		}
		// Fulfill the entity component data count
		std::memcpy(&ecsData[entityDataCountIndex], &entityDataCount, sizeof(size_t));

		// Write child entities
		for each (const auto & child in entity->m_children)
			writeOutEntity(child);
	};
	for each (const auto & entity in m_entities)
		writeOutEntity(entity);

	// Write ecsData to disk
	std::fstream mapFile(Engine::Get_Current_Dir() + "\\Maps\\a.bmap", std::ios::binary | std::ios::out);
	if (!mapFile.is_open())
		m_engine->getManager_Messages().error("Cannot write the binary map file to disk!");
	else
		mapFile.write(ecsData.data(), (std::streamsize)ecsData.size());
	mapFile.close();
}

void World_Module::unloadWorld()
{
	for (auto it = m_components.begin(); it != m_components.end(); ++it) {
		auto typeSize = BaseECSComponent::getTypeSize(it->first);
		auto freefn = BaseECSComponent::getTypeFreeFunction(it->first);
		for (size_t i = 0; i < it->second.size(); i += typeSize)
			freefn((BaseECSComponent*)&it->second[i]);
		it->second.clear();
	}
	m_components.clear();

	std::function<void(ecsEntity*)> deleteEntity = [&](ecsEntity* entity) {
		for each (auto & child in entity->m_children)
			deleteEntity(child);
		delete entity;
	};
	for each (auto & entity in m_entities) 
		deleteEntity(entity);	
	m_entities.clear();

	// Signal that the last map has unloaded
	notifyListeners(unloaded);
}

void World_Module::addLevelListener(const std::shared_ptr<bool>& alive, const std::function<void(const WorldState&)>& func)
{
	m_notifyees.push_back(std::make_pair(alive, func));
}

ecsEntity* World_Module::makeEntity(BaseECSComponent** entityComponents, const int* componentIDs, const size_t& numComponents, const std::string& name, ecsEntity * parent)
{
	auto* newEntity = new ecsEntity();
	for (size_t i = 0; i < numComponents; ++i) {
		if (!BaseECSComponent::isTypeValid(componentIDs[i])) {
			m_engine->getManager_Messages().error("ECS Error: attempted to make an unsupported component, cancelling entity creation...\r\n");
			delete newEntity;
			return NULL_ENTITY_HANDLE;
		}
		addComponentInternal(newEntity, componentIDs[i], entityComponents[i]);
	}
	
	newEntity->m_name = name;
	newEntity->m_entityIndex = (int)m_entities.size();
	m_entities.push_back(newEntity);

	// Parent last
	if (parent)
		parentEntity(parent, newEntity);
	return newEntity;
}

void World_Module::removeEntity(ecsEntity * entity)
{
	auto* root = &m_entities;
	const auto childIndex = size_t(entity->m_entityIndex);

	// Check if the entity has a parent
	if (auto * entityParent = entity->m_parent)
		root = &entityParent->m_children;

	// Delete the components and then the entity
	for (auto & [id, createFn] : entity->m_components)
		deleteComponent(id, createFn);
	delete root->at(childIndex);

	// Swap the entity pointer with the end of the root, then pop it off
	root->at(childIndex) = root->at(root->size() - 1u);
	root->at(childIndex)->m_entityIndex = int(childIndex);
	root->pop_back();
}

std::vector<ecsEntity*> World_Module::getEntities()
{
	return m_entities;
}

void World_Module::parentEntity(ecsEntity* parentEntity, ecsEntity* childEntity)
{
	// Validate input parameters
	if ((!parentEntity && !childEntity) || parentEntity == childEntity)
		return;

	auto * root = &m_entities, * newRoot = parentEntity ? &parentEntity->m_children : &m_entities;
	const auto childIndex = size_t(childEntity->m_entityIndex);

	// Check if the entity has a parent
	if (auto * entityParent = childEntity->m_parent)
		root = &entityParent->m_children;

	// Swap the entity pointer with the end of the root, then remove it from the list
	root->at(childIndex) = root->at(root->size() - 1u);
	root->at(childIndex)->m_entityIndex = int(childIndex);
	root->pop_back();

	// Make this child a child of the new parent, change its index
	childEntity->m_parent = parentEntity;
	childEntity->m_entityIndex = newRoot->size();
	newRoot->push_back(childEntity);
}

void World_Module::unparentEntity(ecsEntity* entity)
{
	// Move entity up tree, making it a child of its old parent's parent
	if (auto & parent = entity->m_parent)
		parentEntity(parent->m_parent, entity);
}

void World_Module::updateSystems(ECSSystemList& systems, const float& deltaTime)
{
	for (size_t i = 0; i < systems.size(); ++i)
		updateSystem(systems[i], deltaTime);
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

void World_Module::processLevel()
{
	if (m_level->existsYet()) {
		const std::function<void(const LevelStruct_Entity&, ecsEntity * parent)> addEntity = [&](const LevelStruct_Entity& entity, ecsEntity * parent) {
			// Retrieve all of this entity's components
			std::vector<BaseECSComponent*> components;
			std::vector<int> ids;
			for each (auto & component in entity.components) {
				if (const auto & [templateComponent, componentID, componentSize] = BaseECSComponent::findTemplate(component.type.data()); templateComponent != nullptr) {
					// Clone the template component completely, then fill in the serialized data
					auto* castedComponent = templateComponent->clone();
					if (component.data.size())
						castedComponent->load(component.data);
					castedComponent->entity = NULL_ENTITY_HANDLE;

					components.push_back(castedComponent);
					ids.push_back(componentID);
				}
			}

			// Make an entity out of the available components			
			auto * thisEntity = makeEntity(components.data(), ids.data(), components.size(), entity.name, parent);

			// Delete temporary components
			for each (auto * component in components)
				delete component;

			// Make child entities
			for each (const auto & child in entity.children)
				addEntity(child, thisEntity);
		};
		for each (const auto & entity in m_level->m_entities)
			addEntity(entity, nullptr);
	}
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
	auto mem_array = m_components[componentID];
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

bool World_Module::addComponentInternal(ecsEntity* entity, const int& componentID, BaseECSComponent* component)
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
			return (BaseECSComponent*)&mem_array[entityComponents[i].second];
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