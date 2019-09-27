#pragma once
#ifndef ECS_WORLD_H
#define ECS_WORLD_H

#include "Modules/ECS/ecsComponent.h"
#include "Modules/ECS/ecsEntity.h"
#include "Modules/ECS/ecsSystem.h"


/** A set of ecs entities and components forming a single level. */
class ecsWorld {
public:
	// Public (de)Constructors
	/** Destroy this ECS World. */
	~ecsWorld();
	/** Construct an empty ECS World. */
	inline ecsWorld() = default;
	/** Construct an ECS world from a serial data buffer. */
	ecsWorld(const std::vector<char>& data);


	// Public Methods
	/** Generate a universally unique identifier for entities or components.
	@return						a new ID. */
	static ecsHandle generateUUID();
	/** Create an entity from a list of input components.
	@param	components			array of component pointers, whom will be hard copied.
	@param	numComponents		the number of components in the array.
	@param	name				optional entity name, more for use in the level editor.
	@param	UUID				optional entity UUID, if empty will auto-generate.
	@param	parentUUID			optional parent entity UUID, if not at the level root.
	@return						handle to the entity on success, empty on failure. */
	ecsHandle makeEntity(ecsBaseComponent** components, const size_t& numComponents, const std::string& name = "Entity", const ecsHandle& UUID = ecsHandle(), const ecsHandle& parentUUID = ecsHandle());
	/** Remove an entity.
	@param	entityHandle		handle to the entity to be removed. */
	void removeEntity(const ecsHandle& entityHandle);
	/** Adds a component to an entity.
	@param	entityHandle		handle to the entity to add the component to.
	@param	component			the component being added.
	@return						true if the component was added successfully, false otherwise (e.g. component ID already present in entity) */
	bool addComponent(const ecsHandle& entityHandle, const ecsBaseComponent* component);
	/** Adds a component to an entity.
	@param	entityHandle		handle to the entity to add the component to.
	@param	component			the component being added.
	@return						true if the component was added successfully, false otherwise (e.g. component ID already present in entity) */
	bool addComponent(const ecsHandle& entityHandle, const ComponentID& componentID, const ecsBaseComponent* component = nullptr);
	/** Removes a component from an entity.
	@param	entityHandle		handle to the entity to remove the component from.
	@param	<T>					the category of component being removed.
	@return						true on successful removal, false otherwise. */
	template <typename T>
	inline bool removeComponent(const ecsHandle& entityHandle) {
		return removeComponent(entityHandle, T::m_ID);
	}
	/** Remove a specific component from an entity and the world.
	@param	entityHandle		handle to the entity to remove the component from.
	@param	componentID			the runtime ID identifying the component class.
	@return						true on successful removal, false otherwise. */
	bool removeComponent(const ecsHandle& entityHandle, const ComponentID& componentID);
	/** Retrieve a component.
	@param	entityHandle		handle to the entity to retrieve from.
	@param	<T>					the category of component being retrieved.
	@return						the specific component of the type requested on success, nullptr otherwise. */
	template <typename T>
	inline T* getComponent(const ecsHandle& entityHandle) {
		return (T*)getComponent(entityHandle, T::m_ID);
	}
	/** Retrieve a component.
	@param	entityHandle		handle to the entity to retrieve from.
	@param	componentID			the runtime ID identifying the component class.
	@return						the specific component on success, nullptr otherwise. */
	inline ecsBaseComponent* getComponent(const ecsHandle& entityHandle, const ComponentID& componentID) {
		if (auto* entity = getEntity(entityHandle))
			return getComponent(entity->m_components, m_components[componentID], componentID);
		return nullptr;
	}
	/** Retrieve the component from an entity matching the class specified.
	@param	entityComponents	the array of entity component IDS.
	@param	array				the array of component data.
	@param	componentID			the class ID of the component.
	@return						the component pointer matching the ID specified. */
	ecsBaseComponent* getComponent(std::vector<std::pair<ComponentID, int>>& entityComponents, ComponentDataSpace& array, const ComponentID& componentID);
	/** Try to find a list of entities matching the UUID's provided.
	@param	UUIDs				list of target entity UUID's
	@return						list of pointers to the found entities. Dimensions may not match input list (nullptrs omitted) */
	std::vector<ecsEntity*> getEntities(const std::vector<ecsHandle>& uuids);
	/** Try to find an entity matching the UUID provided.
	@param	UUID				the target entity's UUID.
	@return						pointer to the found entity on success, nullptr on failure. */
	ecsEntity* getEntity(const ecsHandle& uuid);
	/** Retrieve the top-level root of the map.
	@return						a vector of all level entities. */
	std::vector<ecsHandle> getEntityHandles(const ecsHandle& root = ecsHandle());
	/** Parent an entity to another entity.
	@param	parentEntity		the handle to the desired parent entity.
	@param	childEntity			the handle to the desired child entity. */
	void parentEntity(const ecsHandle& parentEntity, const ecsHandle& childEntity);
	/** Strip a child entity of its parent.
	@param	childEntity			handle to the child entity, whom will be stripped of its parent. */
	void unparentEntity(const ecsHandle& childEntity);
	/** Serialize a specific set of entities to a char vector.
	@param	entityHandles		the set of entities identified by their handles to serialize.
	@return						char vector containing serialized entity data. */
	std::vector<char> serializeEntities(const std::vector<ecsHandle>& entityHandles);
	/** Serialize a specific entity to a char vector.
	@param	entityHandle		the entity to serialize.
	@return						char vector containing serialized entity data. */
	std::vector<char> serializeEntity(const ecsHandle& entityHandle);
	/** Deserialize an entity from a char array.
	@param	data				previously serialized entity data.
	@param	dataSize			the size of the data in bytes (sizeof(char) * elements).
	@param	dataRead			reference to number of elements or bytes read in data so far.
	@param	parentHandle		optional handle to parent entity, designed to be called recursively if entity has children.
	@param	desiredHandle		optional specific handle to use, if empty will use handle held in data stream.
	@return						a handle and a pointer pair to the entity created. */
	std::pair<ecsHandle, ecsEntity*> deserializeEntity(const char* data, const size_t& dataSize, size_t& dataRead, const ecsHandle& parentHandle = ecsHandle(), const ecsHandle& desiredHandle = ecsHandle());
	/** Try to find a component ID based on the component ID.
	@param	name				the component class name to search for.
	@return						optional component ID on success, nullptr on failure. */
	std::optional<ComponentID> nameToComponentID(const char* name);
	/** Search for a component template with a matching class name.
	@param	name				the component class name to search for.
	@return						shared pointer to the a new component with a matching class name if successfull, nullptr otherwise. */
	std::shared_ptr<ecsBaseComponent> makeComponentType(const char* name);
	/** Update the components of all systems provided.
	@param	systems				the systems to update.
	@param	deltaTime			the delta time. */
	void updateSystems(ecsSystemList& systems, const float& deltaTime);
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	void updateSystem(ecsBaseSystem* system, const float& deltaTime);
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	void updateSystem(const std::shared_ptr<ecsBaseSystem>& system, const float& deltaTime);
	/** Update the components of a single system.
	@param	deltaTime			the delta time.
	@param	componentTypes		list of component types to retrieve.
	@param	func				lambda function serving as a system. */
	void updateSystem(const float& deltaTime, const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes, const std::function<void(const float&, const std::vector<std::vector<ecsBaseComponent*>>&)>& func);


private:
	// Private Methods
	/** Check if a given component ID has been previously registered and deemed valid.
	@param	componentID			the component ID to verify.
	@return						true if valid and registered, false otherwise. */
	static bool isComponentIDValid(const ComponentID& componentID);
	/** Delete a component matching the category ID supplied, at the given index.
	@param	componentID			the component class/category ID.
	@param	index				the component index to delete. */
	void deleteComponent(const ComponentID& componentID, const ComponentID& index);
	/** Retrieve the components relevant to an ecs system.
	@param	componentTypes		list of component types to retrieve. */
	std::vector<std::vector<ecsBaseComponent*>> getRelevantComponents(const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes);
	/** Find the least common component.
	@param	componentTypes		the component types. */
	size_t findLeastCommonComponent(const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes);


	// Private Attributes
	ComponentMap m_components;
	EntityMap m_entities;
};

#endif // ECS_WORLD_H
