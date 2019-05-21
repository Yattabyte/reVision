#include "Utilities/ECS/ecs.h"


ECS::~ECS()
{
	for (std::map<uint32_t, std::vector<uint8_t>>::iterator it = m_components.begin(); it != m_components.end(); ++it) {
		size_t typeSize = BaseECSComponent::getTypeSize(it->first);
		ECSComponentFreeFunction freefn = BaseECSComponent::getTypeFreeFunction(it->first);
		for (size_t i = 0; i < it->second.size(); i += typeSize) 
			freefn((BaseECSComponent*)&it->second[i]);		
	}

	for (size_t i = 0; i < m_entities.size(); ++i) 
		delete m_entities[i];
}

EntityHandle ECS::makeEntity(BaseECSComponent ** entityComponents, const uint32_t * componentIDs, const size_t & numComponents)
{
	std::pair<uint32_t, std::vector<std::pair<uint32_t, uint32_t> > >* newEntity = new std::pair<uint32_t, std::vector<std::pair<uint32_t, uint32_t> > >();
	EntityHandle handle = (EntityHandle)newEntity;
	for (size_t i = 0; i < numComponents; ++i) {
		// Check if componentID is actually valid
		if (!BaseECSComponent::isTypeValid(componentIDs[i])) {
			// todo report error
			delete newEntity;
			return NULL_ENTITY_HANDLE;
		}
		addComponentInternal(handle, newEntity->second, componentIDs[i], entityComponents[i]);
	}

	newEntity->first = (uint32_t)m_entities.size();
	m_entities.push_back(newEntity);
	return handle;
}

void ECS::removeEntity(const EntityHandle & handle)
{
	std::vector<std::pair<uint32_t, uint32_t>> & entity = handleToEntity(handle);
	for (size_t i = 0; i < entity.size(); ++i) 
		deleteComponent(entity[i].first, entity[i].second);

	const uint32_t destIndex = handleToEntityIndex(handle);
	const uint32_t srcIndex = (uint32_t)m_entities.size() - 1u;
	delete m_entities[destIndex];
	m_entities[destIndex] = m_entities[srcIndex];
	m_entities[destIndex]->first = destIndex;
	m_entities.pop_back();
}

void ECS::registerConstructor(const char * name, BaseECSComponentConstructor * constructor)
{
	if (m_constructorMap.find(name))
		delete m_constructorMap[name];
	m_constructorMap[name] = constructor;
}

const Component_and_ID ECS::constructComponent(const char * typeName, const std::vector<std::any>& parameters)
{
	if (m_constructorMap.find(typeName))
		return m_constructorMap[typeName]->construct(parameters);
	return Component_and_ID();
}

void ECS::updateSystems(ECSSystemList & systems, const float & deltaTime)
{
	for (size_t i = 0; i < systems.size(); ++i) 
		updateSystem(systems[i], deltaTime);	
}

void ECS::updateSystem(BaseECSSystem * system, const float & deltaTime)
{
	const std::vector<uint32_t> & componentTypes = system->getComponentTypes();
	if (componentTypes.size() == 1u) {
		const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[0]);
		const std::vector<uint8_t>& mem_array = m_components[componentTypes[0]];
		std::vector< std::vector<BaseECSComponent*> > components(mem_array.size() / typeSize);
		for (size_t j = 0, k = 0; j < mem_array.size(); j += typeSize, ++k)
			components[k].push_back((BaseECSComponent*)&mem_array[j]);
		if (components.size())
			system->updateComponents(deltaTime, components);
	}
	else
		updateSystemWithMultipleComponents(system, deltaTime, componentTypes);
}

void ECS::purge()
{
	for (size_t x = 0; x < m_entities.size(); ++x)
		removeEntity(m_entities[x]);

	m_entities.clear();
	m_components.clear();
}

void ECS::deleteComponent(const uint32_t & componentID, const uint32_t & index)
{
	std::vector<uint8_t> mem_array = m_components[componentID];
	ECSComponentFreeFunction freefn = BaseECSComponent::getTypeFreeFunction(componentID);
	const size_t typeSize = BaseECSComponent::getTypeSize(componentID);
	const size_t srcIndex = mem_array.size() - typeSize;

	BaseECSComponent * srcComponent = (BaseECSComponent*)&mem_array[srcIndex];
	BaseECSComponent * destComponent =(BaseECSComponent*)&mem_array[index];
	freefn(destComponent);

	if (index == (uint32_t)srcIndex) {
		mem_array.resize(srcIndex);
		return;
	}
	std::memcpy(destComponent, srcComponent, typeSize);

	// Update references
	std::vector<std::pair<uint32_t, uint32_t>> & srcComponents = handleToEntity(srcComponent->entity);
	for (size_t i = 0; i < srcComponents.size(); ++i) {
		if (componentID == srcComponents[i].first && (uint32_t)srcIndex == srcComponents[i].second) {
			srcComponents[i].second = index;
			break;
		}
	}
	mem_array.resize(srcIndex);
}

void ECS::addComponentInternal(EntityHandle handle, std::vector<std::pair<uint32_t, uint32_t>>& entity, const uint32_t & componentID, BaseECSComponent * component)
{
	ECSComponentCreateFunction createfn = BaseECSComponent::getTypeCreateFunction(componentID);
	std::pair<uint32_t, uint32_t> newPair;
	newPair.first = componentID;
	newPair.second = createfn(m_components[componentID], handle, component);
	entity.push_back(newPair);
}

bool ECS::removeComponentInternal(EntityHandle handle, const uint32_t & componentID)
{
	std::vector<std::pair<uint32_t, uint32_t>> & entityComponents = handleToEntity(handle);
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

BaseECSComponent * ECS::getComponentInternal(std::vector<std::pair<uint32_t, uint32_t>> & entityComponents, std::vector<uint8_t> & mem_array, const uint32_t & componentID)
{
	for (size_t i = 0; i < entityComponents.size(); ++i) 
		if (componentID == entityComponents[i].first) 
			return (BaseECSComponent*)&mem_array[entityComponents[i].second];
	return nullptr;
}

void ECS::updateSystemWithMultipleComponents(BaseECSSystem * system, const float & deltaTime, const std::vector<uint32_t>& componentTypes)
{
	std::vector<BaseECSComponent*> componentParam(componentTypes.size());
	std::vector<std::vector<uint8_t>*> componentArrays(componentTypes.size());
	const std::vector<uint32_t> & componentFlags = system->getComponentFlags();
	for (size_t i = 0; i < componentTypes.size(); ++i) 
		componentArrays[i] = &m_components[componentTypes[i]];	

	const size_t minSizeIndex = findLeastCommonComponent(componentTypes, componentFlags);

	const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[minSizeIndex]);
	const std::vector<uint8_t> & array = *componentArrays[minSizeIndex];
	std::vector< std::vector<BaseECSComponent*> > componentParamList;
	componentParamList.reserve(array.size() / typeSize);
	for (size_t i = 0; i <  array.size(); i += typeSize) {
		componentParam[minSizeIndex] = (BaseECSComponent*)&array[i];
		std::vector<std::pair<uint32_t, uint32_t> > & entityComponents = handleToEntity(componentParam[minSizeIndex]->entity);

		bool isValid = true;
		for (size_t j = 0; j < componentTypes.size(); ++j) {
			if (j == minSizeIndex)
				continue;
			componentParam[j] = getComponentInternal(entityComponents, *componentArrays[j], componentTypes[j]);
			if ( (componentParam[j] == nullptr && (componentFlags[j] & BaseECSSystem::FLAG_REQUIRED)) ) {
				isValid = false;
				break;
			}
		}
		if (isValid) 
			componentParamList.push_back(componentParam);		
	}
	if (componentParamList.size())
		system->updateComponents(deltaTime, componentParamList);
}

size_t ECS::findLeastCommonComponent(const std::vector<uint32_t>& componentTypes, const std::vector<uint32_t> & componentFlags)
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
