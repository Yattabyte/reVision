#include "ECS\ecs.h"


ECS::~ECS()
{
	for (std::map<uint32_t, std::vector<uint8_t>>::iterator it = components.begin(); it != components.end(); ++it) {
		size_t typeSize = BaseECSComponent::getTypeSize(it->first);
		ECSComponentFreeFunction freefn = BaseECSComponent::getTypeFreeFunction(it->first);
		for (size_t i = 0; i < it->second.size(); i += typeSize) 
			freefn((BaseECSComponent*)&it->second[i]);		
	}

	for (size_t i = 0; i < entities.size(); ++i) 
		delete entities[i];
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

	newEntity->first = (uint32_t)entities.size();
	entities.push_back(newEntity);
	return handle;
}

void ECS::removeEntity(const EntityHandle & handle)
{
	std::vector<std::pair<uint32_t, uint32_t>> & entity = handleToEntity(handle);
	for (size_t i = 0; i < entity.size(); ++i) 
		deleteComponent(entity[i].first, entity[i].second);

	const uint32_t destIndex = handleToEntityIndex(handle);
	const uint32_t srcIndex = (uint32_t)entities.size() - 1u;
	delete entities[destIndex];
	entities[destIndex] = entities[srcIndex];
	entities[destIndex]->first = destIndex;
	entities.pop_back();
}

void ECS::updateSystems(ECSSystemList & systems, const float & deltaTime)
{
	for (size_t i = 0; i < systems.size(); ++i) {
		const std::vector<uint32_t> & componentTypes = systems[i]->getComponentTypes();
		if (componentTypes.size() == 1u) {
			const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[0]);
			const std::vector<uint8_t>& mem_array = components[componentTypes[0]];
			std::vector< std::vector<BaseECSComponent*> > components(mem_array.size() / typeSize);
			for (size_t j = 0, k = 0; j < mem_array.size(); j += typeSize, ++k) 
				components[k].push_back((BaseECSComponent*)&mem_array[j]);
			if (components.size())
				systems[i]->updateComponents(deltaTime, components);
		}
		else 
			updateSystemWithMultipleComponents(systems, i, deltaTime, componentTypes);
	}
}

void ECS::deleteComponent(const uint32_t & componentID, const uint32_t & index)
{
	std::vector<uint8_t> mem_array = components[componentID];
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
	newPair.second = createfn(components[componentID], handle, component);
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

void ECS::updateSystemWithMultipleComponents(ECSSystemList & systems, const size_t & index, const float & deltaTime, const std::vector<uint32_t>& componentTypes)
{
	std::vector<BaseECSComponent*> componentParam(componentTypes.size());
	std::vector<std::vector<uint8_t>*> componentArrays(componentTypes.size());
	const std::vector<uint32_t> & componentFlags = systems[index]->getComponentFlags();
	for (size_t i = 0; i < componentTypes.size(); ++i) 
		componentArrays[i] = &components[componentTypes[i]];	

	const size_t minSizeIndex = findLeastCommonComponent(componentTypes, componentFlags);

	const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[minSizeIndex]);
	const std::vector<uint8_t> & array = *componentArrays[minSizeIndex];
	std::vector< std::vector<BaseECSComponent*> > components;
	components.reserve(array.size() / typeSize);
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
			components.push_back(componentParam);		
	}
	if (components.size())
		systems[index]->updateComponents(deltaTime, components);
}

size_t ECS::findLeastCommonComponent(const std::vector<uint32_t>& componentTypes, const std::vector<uint32_t> & componentFlags)
{
	size_t minSize = (size_t)-1;
	size_t minIndex = (size_t)-1;
	for (size_t i = 0; i < componentTypes.size(); ++i) {
		if ((componentFlags[i] & BaseECSSystem::FLAG_OPTIONAL) != 0)
			continue;
		const size_t typeSize = BaseECSComponent::getTypeSize(componentTypes[i]);
		const size_t size = components[componentTypes[i]].size() / typeSize;
		if (size <= minSize) {
			minSize = size;
			minIndex = i;
		}
	}
	return minIndex;
}
