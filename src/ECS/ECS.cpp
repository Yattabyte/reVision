#include "ECS\ecs.h"


ECS::~ECS()
{
	for (std::map<unsigned int, std::vector<unsigned int>>::iterator it = components.begin(); it != components.end(); ++it) {
		size_t typeSize = BaseECSComponent::getTypeSize(it->first);
		ECSComponentFreeFunction freefn = BaseECSComponent::getTypeFreeFunction(it->first);
		for (size_t i = 0; i < it->second.size(); i += typeSize) 
			freefn((BaseECSComponent*)&it->second[i]);		
	}

	for (unsigned int i = 0; i < entities.size(); ++i) 
		delete entities[i];
}

EntityHandle ECS::makeEntity(BaseECSComponent ** entityComponents, const unsigned int * componentIDs, const size_t & numComponents)
{
	std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int> > >* newEntity = new std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int> > >();
	EntityHandle handle = (EntityHandle)newEntity;
	for (unsigned int i = 0; i < numComponents; ++i) {
		// Check if componentID is actually valid
		if (!BaseECSComponent::isTypeValid(componentIDs[i])) {
			// todo report error
			delete newEntity;
			return NULL_ENTITY_HANDLE;
		}
		addComponentInternal(handle, newEntity->second, componentIDs[i], entityComponents[i]);
	}

	newEntity->first = entities.size();
	entities.push_back(newEntity);
	return handle;
}

void ECS::removeEntity(const EntityHandle & handle)
{
	std::vector<std::pair<unsigned int, unsigned int>> & entity = handleToEntity(handle);
	for (unsigned int i = 0; i < entity.size(); ++i) 
		deleteComponent(entity[i].first, entity[i].second);

	const unsigned int destIndex = handleToEntityIndex(handle);
	const unsigned int srcIndex = entities.size() - 1;
	delete entities[destIndex];
	entities[destIndex] = entities[srcIndex];
	entities[destIndex]->first = destIndex;
	entities.pop_back();
}

void ECS::updateSystems(ECSSystemList & systems, const float & deltaTime)
{
	for (size_t i = 0; i < systems.size(); ++i) {
		const std::vector<unsigned int>& componentTypes = systems[i]->getComponentTypes();
		if (componentTypes.size() == 1) {
			const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[0]);
			const std::vector<unsigned int>& array = components[componentTypes[0]];
			std::vector< std::vector<BaseECSComponent*> > components(array.size() / typeSize);
			for (size_t j = 0, k = 0; j < array.size(); j += typeSize, ++k) 
				components[k].push_back((BaseECSComponent*)&array[j]);
			
			systems[i]->updateComponents(deltaTime, components);
		}
		else 
			updateSystemWithMultipleComponents(systems, i, deltaTime, componentTypes);
	}
}

void ECS::deleteComponent(const unsigned int & componentID, const unsigned int & index)
{
	std::vector<unsigned int> array = components[componentID];
	ECSComponentFreeFunction freefn = BaseECSComponent::getTypeFreeFunction(componentID);
	const size_t typeSize = BaseECSComponent::getTypeSize(componentID);
	const size_t srcIndex = array.size() - typeSize;

	BaseECSComponent * srcComponent = (BaseECSComponent*)&array[srcIndex];
	BaseECSComponent * destComponent =(BaseECSComponent*)&array[index];
	freefn(destComponent);

	if (index == srcIndex) {
		array.resize(srcIndex);
		return;
	}
	std::memcpy(destComponent, srcComponent, typeSize);

	// Update references
	std::vector<std::pair<unsigned int, unsigned int>> & srcComponents = handleToEntity(srcComponent->entity);
	for (unsigned int i = 0; i < srcComponents.size(); ++i) {
		if (componentID == srcComponents[i].first && srcIndex == srcComponents[i].second) {
			srcComponents[i].second = index;
			break;
		}
	}
	array.resize(srcIndex);
}

void ECS::addComponentInternal(EntityHandle handle, std::vector<std::pair<unsigned int, unsigned int>>& entity, const unsigned int & componentID, BaseECSComponent * component)
{
	ECSComponentCreateFunction createfn = BaseECSComponent::getTypeCreateFunction(componentID);
	std::pair<unsigned int, unsigned int> newPair;
	newPair.first = componentID;
	newPair.second = createfn(components[componentID], handle, component);
	entity.push_back(newPair);
}

bool ECS::removeComponentInternal(EntityHandle handle, const unsigned int & componentID)
{
	std::vector<std::pair<unsigned int, unsigned int>> & entityComponents = handleToEntity(handle);
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

BaseECSComponent * ECS::getComponentInternal(std::vector<std::pair<unsigned int, unsigned int>> & entityComponents, std::vector<unsigned int> & array, const unsigned int & componentID)
{
	for (unsigned int i = 0; i < entityComponents.size(); ++i) 
		if (componentID == entityComponents[i].first) 
			return (BaseECSComponent*)&array[entityComponents[i].second];
	return nullptr;
}

void ECS::updateSystemWithMultipleComponents(ECSSystemList & systems, const unsigned int & index, const float & deltaTime, const std::vector<unsigned int>& componentTypes)
{
	std::vector<BaseECSComponent*> componentParam(componentTypes.size());
	std::vector<std::vector<unsigned int>*> componentArrays(componentTypes.size());
	const std::vector<unsigned int> & componentFlags = systems[index]->getComponentFlags();
	for (unsigned int i = 0; i < componentTypes.size(); ++i) 
		componentArrays[i] = &components[componentTypes[i]];	

	unsigned int minSizeIndex = findLeastCommonComponent(componentTypes, componentFlags);

	const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[minSizeIndex]);
	const std::vector<unsigned int> & array = *componentArrays[minSizeIndex];
	std::vector< std::vector<BaseECSComponent*> > components;
	components.reserve(array.size() / typeSize);
	for (size_t i = 0; i <  array.size(); i += typeSize) {
		componentParam[minSizeIndex] = (BaseECSComponent*)&array[i];
		std::vector<std::pair<unsigned int, unsigned int> > & entityComponents = handleToEntity(componentParam[minSizeIndex]->entity);

		bool isValid = true;
		for (unsigned int j = 0; j < componentTypes.size(); ++j) {
			if (j == minSizeIndex)
				continue;
			componentParam[j] = getComponentInternal(entityComponents, *componentArrays[j], componentTypes[j]);
			if ( (componentParam[j] == nullptr && (componentFlags[j] & BaseECSSystem::FLAG_REQUIRED)) ) {
				isValid = false;
				break;
			}
		}
		if (isValid) 
			components.push_back(componentParam);		
	}
	systems[index]->updateComponents(deltaTime, components);
}

unsigned int ECS::findLeastCommonComponent(const std::vector<unsigned int>& componentTypes, const std::vector<unsigned int> & componentFlags)
{
	unsigned int minSize = (unsigned int)-1;
	unsigned int minIndex = (unsigned int)-1;
	for (size_t i = 0; i < componentTypes.size(); ++i) {
		if ((componentFlags[i] & BaseECSSystem::FLAG_OPTIONAL) != 0)
			continue;
		const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[i]);
		unsigned int size = components[componentTypes[i]].size() / typeSize;
		if (size <= minSize) {
			minSize = size;
			minIndex = i;
		}
	}
	return minIndex;
}
