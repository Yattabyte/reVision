#include "Modules/World/World_M.h"
#include "Modules/World/ECS/components.h"
#include "Utilities/IO/Level_IO.h"
#include "Engine.h"
#include <fstream>
#include <filesystem>


void World_Module::initialize(Engine * engine)
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

void World_Module::frameTick(const float & deltaTime)
{
	auto & assetManager = m_engine->getManager_Assets();

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

void World_Module::loadWorld(const std::string & mapName)
{
	unloadWorld();

	// Signal that a new map is begining to load
	notifyListeners(startLoading);
	
	m_level = Shared_Level(m_engine, mapName);
	m_level->addCallback(m_aliveIndicator, std::bind(&World_Module::processLevel, this));
}

void World_Module::saveWorld(const std::string & mapName)
{
	std::vector<char> ecsData;
	for each (const auto & entity in m_entities) {
		// Remember the beginning spot for this entity, we will update this index with the total entity data size
		const auto entityDataCount_Offset = ecsData.size();
		ecsData.resize(entityDataCount_Offset + sizeof(size_t));
		size_t entityDataCount(0ull);
		const auto & handle = (EntityHandle)entity;
		for (const auto &[componentID, createFunc] : entity->second) {
			const auto componentData = getComponentInternal(handleToEntity(entity), m_components[componentID], componentID)->save();
			const auto componentDataSize = componentData.size();
			entityDataCount += sizeof(size_t) + componentDataSize;

			// Write component data size + component data to ecs data
			const auto oldDataSize = ecsData.size();
			ecsData.resize(oldDataSize + sizeof(size_t) + componentDataSize);
			std::memcpy(&ecsData[oldDataSize], &componentDataSize, sizeof(size_t));
			std::memcpy(&ecsData[oldDataSize + sizeof(size_t)], &componentData[0], componentDataSize);
		}

		// modify entity data count at offset spot to be total data
		std::memcpy(&ecsData[entityDataCount_Offset], &entityDataCount, sizeof(size_t));
	}

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
		size_t typeSize = BaseECSComponent::getTypeSize(it->first);
		ECSComponentFreeFunction freefn = BaseECSComponent::getTypeFreeFunction(it->first);
		for (size_t i = 0; i < it->second.size(); i += typeSize)
			freefn((BaseECSComponent*)&it->second[i]);
		it->second.clear();
	}
	m_components.clear();

	for (size_t i = 0; i < m_entities.size(); ++i)
		delete m_entities[i];
	m_entities.clear();
	
	// Signal that the last map has unloaded
	notifyListeners(unloaded);
}

void World_Module::addLevelListener(const std::shared_ptr<bool> & alive, const std::function<void(const WorldState&)> & func)
{
	m_notifyees.push_back(std::make_pair(alive, func));
}

EntityHandle World_Module::makeEntity(BaseECSComponent ** entityComponents, const int * componentIDs, const size_t & numComponents)
{
	auto * newEntity = new std::pair<int, std::vector<std::pair<int, int> > >();
	EntityHandle handle = (EntityHandle)newEntity;
	for (size_t i = 0; i < numComponents; ++i) {
		// Check if componentID is actually valid
		if (!BaseECSComponent::isTypeValid(componentIDs[i])) {
			m_engine->getManager_Messages().error("ECS Error: attempted to make an unsupported component, cancelling entity creation...\r\n");
			delete newEntity;
			return NULL_ENTITY_HANDLE;
		}
		addComponentInternal(handle, newEntity->second, componentIDs[i], entityComponents[i]);
	}

	newEntity->first = (int)m_entities.size();
	m_entities.push_back(newEntity);
	return handle;
}

void World_Module::removeEntity(const EntityHandle & handle)
{
	std::vector<std::pair<int, int>> & entity = handleToEntity(handle);
	for (size_t i = 0; i < entity.size(); ++i)
		deleteComponent(entity[i].first, entity[i].second);

	const int destIndex = handleToEntityIndex(handle);
	const int srcIndex = (int)m_entities.size() - 1u;
	delete m_entities[destIndex];
	m_entities[destIndex] = m_entities[srcIndex];
	m_entities[destIndex]->first = destIndex;
	m_entities.pop_back();
}

void World_Module::updateSystems(ECSSystemList & systems, const float & deltaTime)
{
	for (size_t i = 0; i < systems.size(); ++i)
		updateSystem(systems[i], deltaTime);
}

void World_Module::updateSystem(BaseECSSystem * system, const float & deltaTime)
{
	if (auto components = getRelevantComponents(system->getComponentTypes(), system->getComponentFlags()); components.size() > 0ull)
		system->updateComponents(deltaTime, components);
}

void World_Module::updateSystem(const float & deltaTime, const std::vector<int> & types, const std::vector<int> & flags, const std::function<void(const float&, const std::vector<std::vector<BaseECSComponent*>>&)>& func)
{
	if (auto components = getRelevantComponents(types, flags); components.size() > 0ull)
		func(deltaTime, components);	
}

void World_Module::processLevel()
{
	if (m_level->existsYet()) {	
		for each (auto & entity in m_level->m_entities) {
			// Retrieve all of this entity's components
			std::vector<BaseECSComponent*> components;
			std::vector<int> ids;
			for each (auto & component in entity.components) {
				if (const auto &[templateComponent, componentID, componentSize] = BaseECSComponent::findTemplate(component.type.data()); templateComponent != nullptr) {
					// Clone the template component completely, then fill in the serialized data
					auto * castedComponent = templateComponent->clone();
					if (component.data.size())
						castedComponent->load(component.data);
					castedComponent->entity = NULL_ENTITY_HANDLE;

					components.push_back(castedComponent);
					ids.push_back(componentID);
				}
			}

			// Make an entity out of the available components
			if (components.size())
				makeEntity(components.data(), ids.data(), components.size());

			// Delete temporary components
			for each (auto * component in components)
				delete component;
		}
	}
}

void World_Module::notifyListeners(const WorldState & state)
{
	// Get all callback functions, and call them if their owners are still alive
	for (int x = 0; x < (int)m_notifyees.size(); ++x) {
		const auto &[alive, func] = m_notifyees[x];
		if (alive && *(alive.get()) == true)
			func(state);
		else {
			m_notifyees.erase(m_notifyees.begin() + x);
			x--;
		}
	}
	m_state = state;
}

void World_Module::deleteComponent(const int & componentID, const int & index)
{
	std::vector<uint8_t> mem_array = m_components[componentID];
	ECSComponentFreeFunction freefn = BaseECSComponent::getTypeFreeFunction(componentID);
	const size_t typeSize = BaseECSComponent::getTypeSize(componentID);
	const size_t srcIndex = mem_array.size() - typeSize;

	BaseECSComponent * srcComponent = (BaseECSComponent*)&mem_array[srcIndex];
	BaseECSComponent * destComponent = (BaseECSComponent*)&mem_array[index];
	freefn(destComponent);

	if (index == (int)srcIndex) {
		mem_array.resize(srcIndex);
		return;
	}
	std::memcpy(destComponent, srcComponent, typeSize);

	// Update references
	std::vector<std::pair<int, int>> & srcComponents = handleToEntity(srcComponent->entity);
	for (size_t i = 0; i < srcComponents.size(); ++i) {
		if (componentID == srcComponents[i].first && (int)srcIndex == srcComponents[i].second) {
			srcComponents[i].second = index;
			break;
		}
	}
	mem_array.resize(srcIndex);
}

void World_Module::addComponentInternal(EntityHandle handle, std::vector<std::pair<int, int>>& entity, const int & componentID, BaseECSComponent * component)
{
	ECSComponentCreateFunction createfn = BaseECSComponent::getTypeCreateFunction(componentID);
	std::pair<int, int> newPair;
	newPair.first = componentID;
	newPair.second = createfn(m_components[componentID], handle, component);
	entity.push_back(newPair);
}

bool World_Module::removeComponentInternal(EntityHandle handle, const int & componentID)
{
	std::vector<std::pair<int, int>> & entityComponents = handleToEntity(handle);
	for (size_t i = 0; i < entityComponents.size(); ++i) {
		if (componentID == entityComponents[i].first) {
			deleteComponent(entityComponents[i].first, entityComponents[i].second);
			const size_t srcIndex = entityComponents.size() - 1;
			const size_t destIndex = i;
			entityComponents[destIndex] = entityComponents[srcIndex];
			entityComponents.pop_back();
			return true;
		}
	}
	return false;
}

BaseECSComponent * World_Module::getComponentInternal(std::vector<std::pair<int, int>> & entityComponents, std::vector<uint8_t> & mem_array, const int & componentID)
{
	for (size_t i = 0; i < entityComponents.size(); ++i)
		if (componentID == entityComponents[i].first)
			return (BaseECSComponent*)&mem_array[entityComponents[i].second];
	return nullptr;
}

std::vector<std::vector<BaseECSComponent*>> World_Module::getRelevantComponents(const std::vector<int>& componentTypes, const std::vector<int>& componentFlags)
{
	std::vector< std::vector<BaseECSComponent*> > components;
	if (componentTypes.size() > 0ull) {
		if (componentTypes.size() == 1u) {
			// Super simple procedure for system with 1 component type
			const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[0]);
			const std::vector<uint8_t>& mem_array = m_components[componentTypes[0]];
			components.resize(mem_array.size() / typeSize);
			for (size_t j = 0, k = 0; j < mem_array.size(); j += typeSize, ++k)
				components[k].push_back((BaseECSComponent*)&mem_array[j]);
		}
		else {
			// More complex procedure for system with > 1 component type
			std::vector<BaseECSComponent*> componentParam(componentTypes.size());
			std::vector<std::vector<uint8_t>*> componentArrays(componentTypes.size());
			for (size_t i = 0; i < componentTypes.size(); ++i)
				componentArrays[i] = &m_components[componentTypes[i]];

			const size_t minSizeIndex = findLeastCommonComponent(componentTypes, componentFlags);
			const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[minSizeIndex]);
			const std::vector<uint8_t> & mem_array = *componentArrays[minSizeIndex];
			components.reserve(mem_array.size() / typeSize); // reserve, not resize, as we component at [i] may be invalid
			for (size_t i = 0; i < mem_array.size(); i += typeSize) {
				componentParam[minSizeIndex] = (BaseECSComponent*)&mem_array[i];
				std::vector<std::pair<int, int> > & entityComponents = handleToEntity(componentParam[minSizeIndex]->entity);

				bool isValid = true;
				for (size_t j = 0; j < componentTypes.size(); ++j) {
					if (j == minSizeIndex)
						continue;
					componentParam[j] = getComponentInternal(entityComponents, *componentArrays[j], componentTypes[j]);
					if ((componentParam[j] == nullptr) && (componentFlags[j] & BaseECSSystem::FLAG_REQUIRED)) {
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

size_t World_Module::findLeastCommonComponent(const std::vector<int>& componentTypes, const std::vector<int> & componentFlags)
{
	size_t minSize = (size_t)-1;
	size_t minIndex = (size_t)-1;
	for (size_t i = 0; i < componentTypes.size(); ++i) {
		if ((componentFlags[i] & BaseECSSystem::FLAG_OPTIONAL) != 0)
			continue;
		const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[i]);
		const size_t size = m_components[componentTypes[i]].size() / typeSize;
		if (size <= minSize) {
			minSize = size;
			minIndex = i;
		}
	}
	return minIndex;
}
