#pragma once
#ifndef WORLD_MODULE_H
#define WORLD_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "Modules/World/ECS/ecsEntity.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Utilities/MappedChar.h"
#include <map>
#include <functional>
#include <vector>


/** A module responsible for the world/level. */
class World_Module : public Engine_Module {
public:
	// Public Enumerations
	const enum WorldState {
		unloaded,
		startLoading,
		finishLoading,
		updated
	};


	// Public (de)Constructors
	/** Destroy this world module. */
	inline ~World_Module() = default;
	/** Construct a world module. */
	inline World_Module() = default;


	// Public Interface Implementations
	virtual void initialize(Engine* engine) override;
	virtual void deinitialize() override;
	virtual void frameTick(const float& deltaTime) override;


	// Public Methods
	/** Loads the world, specified by the map name.
	@param	mapName				the name of the map to load. */
	void loadWorld(const std::string& mapName);
	/** Saves the world with a specified map name.
	@param	mapName				the name of the map to save as. */
	void saveWorld(const std::string& mapName);
	/** Unload the current world. */
	void unloadWorld();
	/** Serialize a specific entity to a char vector.
	@param	entity				the entity to serialize.
	@return						char vector containing serialized entity data. */
	std::vector<char> serializeEntity(ecsEntity* entity);
	/** Deserialize an entity from a char array.
	@param	data				previously serialized entity data.
	@param	dataSize			the size of the data in bytes (sizeof(char) * elements).
	@param	dataRead			reference to number of elements or bytes read in data so far.
	@param	parent				optional pointer to parent entity, designed to be called recursively if entity has children. */
	ecsEntity* deserializeEntity(const char * data, const size_t& dataSize, size_t& dataRead, ecsEntity* parent = nullptr);
	/** Serialize a specific component to a char vector.
	@param	component			the component to serialize.
	@return						char vector containing serialized component data. */
	std::vector<char> serializeComponent(BaseECSComponent* component);
	/** Deserialize a component from a char array.
	@param	data				previously serialized component data.
	@param	dataSize			the size of the data in bytes (sizeof(char) * elements).
	@param	dataRead			reference to number of elements or bytes read in data so far. */
	std::pair<BaseECSComponent*, int> deserializeComponent(const char * data, const size_t& dataSize, size_t& dataRead);

	/** Registers a notification function to be called when the world state changes.
	@param	alive				a shared pointer indicating whether the caller is still alive or not.
	@param	notifier			function to be called on state change. */
	void addLevelListener(const std::shared_ptr<bool>& alive, const std::function<void(const WorldState&)>& func);
	/** Create an entity from a list of input components.
	@param	components			array of component pointers, whom will be hard copied.
	@param	componentIDS		array of component ids.
	@param	numComponents		the number of components in the array.
	@param	name				optional entity name, more for use in the level editor.
	@param	UUID				optional entity UUID, if empty will auto-generate.
	@param	parentEntity		optional parent entity, if not at the level root. */
	ecsEntity * makeEntity(BaseECSComponent** components, const int* componentIDS, const size_t& numComponents, const std::string& name = "Entity", const std::string& UUID = "", ecsEntity * parentEntity = nullptr);
	/** Construct an entity from the array of component references.
	@note Variadic
	@param	...args				all components to use for this entity. */
	template <class...Args>
	inline ecsEntity * makeEntity(Args& ...args) {
		BaseECSComponent* components[] = { &args... };
		const int componentIDS[] = { Args::ID... };
		return makeEntity(components, componentIDS, sizeof...(Args));
	}
	/** Construct an entity from the array of component pointers.
	@note Variadic
	@param	args				all components to use for this entity. */
	template <class...Args>
	inline ecsEntity * makeEntity(Args* ...args) {
		BaseECSComponent* components[] = { args... };
		const int componentIDS[] = { Args::ID... };
		return makeEntity(components, componentIDS, sizeof...(Args));
	}
	/** Remove an entity.
	@param	entity				the entity to remove. */
	void removeEntity(ecsEntity* entity);
	/** Retrieve a list of all level entities. 
	@return						a vector of all level entities. */
	std::vector<ecsEntity*> getEntities();	
	/**
	/** Adds a component to an entity.
	@param	entity				the entity to add the component to.
	@param	component			the component being added.
	@return						true if the component was added successfully, false otherwise (e.g. component ID already present in entity) */
	inline bool addComponent(ecsEntity* entity, const BaseECSComponent* component) {
		return addComponentInternal(entity, component->get_id(), component);
	}
	/** Removes a component from an entity.
	@param	entity				the entity to remove from.
	@param	<BaseECSComponent>	the category of component being removed.
	@return						true on successful removal, false otherwise. */
	template <typename T>
	inline bool removeComponent(ecsEntity* entity) {
		return removeComponent(entity, T::ID);
	}
	/** Remove a specific component from an entity and the world.
	@param	entity				the entity to remove the component from.
	@param	componentID			the runtime ID identifying the component class. */
	inline bool removeComponent(ecsEntity* entity, const int& componentID) {
		return removeComponentInternal(entity, componentID);
	}
	/** Retrieve a component.
	@param	entity				the entity to retrieve from.
	@param	<BaseECSComponent>	the category of component being retrieved. */
	template <typename T>
	inline T* getComponent(ecsEntity* entity) {
		return (T*)getComponentInternal(entity->m_components, m_components[T::ID], T::ID);
	}
	/** Parent an entity to another entity.
	@param	parentEntity		the handle to the desired parent entity.
	@param	childEntity			the handle to the desired child entity. */
	void parentEntity(ecsEntity* parentEntity, ecsEntity* childEntity);
	/** Strip a child entity of its parent. 
	@param	childEntity			handle to the child entity, whom will be stripped of its parent. */
	void unparentEntity(ecsEntity* childEntity);
	/** Try to find an entity matching the UUID provided.
	@param	UUID				the target entity's UUID.
	@return						pointer to the found entity on success, nullptr on failure. */
	ecsEntity* findEntity(const std::string& uuid);
	/** Try to find a list of entities matching the UUID's provided.
	@param	UUIDs				list of target entity UUID's
	@return						list of pointers to the found entities. Dimensions may not match input list (nullptrs omitted) */
	std::vector<ecsEntity*> findEntities(const std::vector<std::string>& uuids);
	/** Update the components of all systems provided.
	@param	systems				the systems to update.
	@param	deltaTime			the delta time. */
	void updateSystems(ECSSystemList& systems, const float& deltaTime);
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	void updateSystem(BaseECSSystem* system, const float& deltaTime);
	/** Update the components of a single system.
	@param	deltaTime			the delta time.
	@param	types				list of component types to retrieve.
	@param	flags				list of flags, designating a component as required or optional.
	@param	func				lambda function serving as a system. */
	void updateSystem(const float& deltaTime, const std::vector<int>& types, const std::vector<int>& flags, const std::function<void(const float&, const std::vector<std::vector<BaseECSComponent*>>&)>& func);


private:
	// Private Methods
	/** Notify all world-listeners of a state change.
	@param	state				the new state to notify listeners of. */
	void notifyListeners(const WorldState& state);
	/** Delete a component matching the category ID supplied, at the given index.
	@param	componentID			the component class/category ID.
	@param	index				the component index to delete. */
	void deleteComponent(const int& componentID, const int& index);
	/** Adds a component to the entity specified.
	@param	entity				the entity
	@param	componentID			the class ID of the component.
	@param	component			the specific component to add to the entity.
	@return						true if the component was added successfully, false otherwise (e.g. component ID already present in entity) */
	bool addComponentInternal(ecsEntity* entity, const int& componentID, const BaseECSComponent* component);
	/** Remove a component from the entity specified.
	@param	handle				the entity handle, to remove the component from.
	@param	componentID			the class ID of the component.
	@return						true on remove success, false otherwise. */
	bool removeComponentInternal(ecsEntity* entity, const int& componentID);
	/** Retrieve the component from an entity matching the class specified.
	@param	entityComponents	the array of entity component IDS.
	@param	array				the array of component data.
	@param	componentID			the class ID of the component.
	@return						the component pointer matching the ID specified. */
	BaseECSComponent* getComponentInternal(std::vector<std::pair<int, int>>& entityComponents, std::vector<uint8_t>& array, const int& componentID);
	/** Retrieve the components relevant to an ecs system.
	@param	componentTypes		list of component types to retrieve.
	@param	componentFlags		list of flags, designating a component as required or optional. */
	std::vector<std::vector<BaseECSComponent*>> getRelevantComponents(const std::vector<int>& componentTypes, const std::vector<int>& componentFlags);
	/** Find the least common component.
	@param	componentTypes		the component types.
	@param	componentFlags		the component flags. */
	size_t findLeastCommonComponent(const std::vector<int>& componentTypes, const std::vector<int>& componentFlags);
	/***/
	static std::string generateUUID();
	/***/
	void validateUIDS();


	// Private Attributes
	bool m_finishedLoading = false;
	std::map<int, std::vector<uint8_t>> m_components;
	std::vector<ecsEntity*> m_entities;
	WorldState m_state = unloaded;
	std::vector<std::pair<std::shared_ptr<bool>, std::function<void(const WorldState&)>>> m_notifyees;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // WORLD_MODULE_h