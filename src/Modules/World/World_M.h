#pragma once
#ifndef WORLD_MODULE_H
#define WORLD_MODULE_H

#include "Modules/Engine_Module.h"
#include "Assets/Level.h"
#include "Modules/World/ECS/ecsComponent.h"
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
	virtual void initialize(Engine * engine) override;
	virtual void deinitialize() override;
	virtual void frameTick(const float & deltaTime) override;


	// Public Methods
	/** Loads the world, specified by the map name.
	@param	mapName		the name of the map to load. */
	void loadWorld(const std::string & mapName);
	/***/
	void saveWorld(const std::string & mapName);
	/** Unload the current world. */
	void unloadWorld();
	/** Registers a notification function to be called when the world state changes.
	@param	alive		a shared pointer indicating whether the caller is still alive or not.
	@param	notifier	function to be called on state change. */
	void addLevelListener(const std::shared_ptr<bool> & alive, const std::function<void(const WorldState&)> & func);
	/** Convert an entity handle to its raw index.
	@param	handle				the entity handle to process.
	@return						raw entity index. */
	inline int handleToEntityIndex(const EntityHandle& handle) {
		return handleToRawType(handle)->first;
	}
	/** Convert an entity handle to its raw data.
	@param	handle				the entity handle to process.
	@return						raw entity data. */
	inline std::vector<std::pair<int, int>>& handleToEntity(const EntityHandle& handle) {
		return handleToRawType(handle)->second;
	}
	/** Construct an entity from the array of components and IDS*/
	EntityHandle makeEntity(BaseECSComponent ** components, const int * componentIDS, const size_t & numComponents, const std::string & name = "Entity");
	/** Construct an entity from the array of component references.
	@note Variadic
	@param	args	all components to use for this entity. */
	template <class...Args>
	inline EntityHandle makeEntity(Args&...args) {
		BaseECSComponent * components[] = { &args... };
		const int componentIDS[] = { Args::ID... };
		return makeEntity(components, componentIDS, sizeof...(Args));
	}
	/** Construct an entity from the array of component pointers.
	@note Variadic
	@param	args	all components to use for this entity. */
	template <class...Args>
	inline EntityHandle makeEntity(Args*...args) {
		BaseECSComponent * components[] = { args... };
		const int componentIDS[] = { Args::ID... };
		return makeEntity(components, componentIDS, sizeof...(Args));
	}
	/** Remove an entity.
	@param	handle	the entity to remove. */
	void removeEntity(const EntityHandle & handle);
	/***/
	std::vector<EntityHandle> getEntities();
	/***/
	std::vector<std::string> & getEntityNames();
	/** Adds a component to an entity.
	@param	entity				the entity to add the component to.
	@param	component			the component being added.
	@return						true if the component was added successfully, false otherwise (e.g. component ID already present in entity) */
	template <typename SpecificComponent>
	inline bool addComponent(const EntityHandle& entity, const SpecificComponent& component) {
		return addComponentInternal(entity, handleToEntity(entity), SpecificComponent::ID, &component);
	}
	/**
	/** Adds a component to an entity.
	@param	entity				the entity to add the component to.
	@param	component			the component being added.
	@return						true if the component was added successfully, false otherwise (e.g. component ID already present in entity) */
	inline bool addComponent(const EntityHandle & entity, BaseECSComponent * component) {
		return addComponentInternal(entity, handleToEntity(entity), component->get_id(), component);
	}
	/** Removes a component from an entity.
	@param	entity				the entity to remove from.
	@param	<BaseECSComponent>	the category of component being removed.
	@return						true on successful removal, false otherwise. */
	template <typename T>
	inline bool removeComponent(const EntityHandle & entity) {
		return removeComponent(entity, T::ID);
	}
	/***/
	inline bool removeComponent(const EntityHandle & entity, const int & componentID) {
		return removeComponentInternal(entity, componentID);
	}
	/** Retrieve a component.
	@param	entity				the entity to retrieve from.
	@param	<BaseECSComponent>	the category of component being retrieved. */
	template <typename T>
	inline T * getComponent(const EntityHandle & entity) {
		return (T*)getComponentInternal(handleToEntity(entity), m_components[T::ID], T::ID);
	}
	/** Update the components of all systems provided.
	@param	systems				the systems to update.
	@param	deltaTime			the delta time. */
	void updateSystems(ECSSystemList & systems, const float & deltaTime);
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	void updateSystem(BaseECSSystem * system, const float & deltaTime);
	/** Update the components of a single system.
	@param	deltaTime			the delta time. 
	@param	types				list of component types to retrieve.
	@param	flags				list of flags, designating a component as required or optional. 
	@param	func				lambda function serving as a system. */
	void updateSystem(const float & deltaTime, const std::vector<int> & types, const std::vector<int> & flags, const std::function<void(const float &, const std::vector<std::vector<BaseECSComponent*>>&)> &func);


private:
	// Private Methods
	/** Process the level asset, generating components and entities. */
	void processLevel();
	/** Notify all world-listeners of a state change. 
	@param	state				the new state to notify listeners of. */
	void notifyListeners(const WorldState & state);
	/** Convert an entity handle to the specific raw type. 
	@param	handle				the entity handle to process.
	@return						raw handle. */
	inline std::pair< int, std::vector<std::pair<int, int> > >* handleToRawType(const EntityHandle & handle) {
		return (std::pair< int, std::vector<std::pair<int, int> > >*)handle;
	}
	/** Delete a component matching the category ID supplied, at the given index. 
	@param	componentID			the component class/category ID.
	@param	index				the component index to delete. */
	void deleteComponent(const int & componentID, const int & index);
	/** Adds a component to the entity specified.
	@param	handle				the entity handle, to add the component to.
	@param	entity				the specific entity data array.
	@param	componentID			the class ID of the component.
	@param	component			the specific component to add to the entity. 
	@return						true if the component was added successfully, false otherwise (e.g. component ID already present in entity) */
	bool addComponentInternal(EntityHandle handle, std::vector<std::pair<int, int>> & entity, const int & componentID, BaseECSComponent * component);
	/** Remove a component from the entity specified.
	@param	handle				the entity handle, to remove the component from.
	@param	componentID			the class ID of the component. 
	@return						true on remove success, false otherwise. */
	bool removeComponentInternal(EntityHandle handle, const int & componentID);
	/** Retrieve the component from an entity matching the class specified.
	@param	entityComponents	the array of entity component IDS.
	@param	array				the array of component data.
	@param	componentID			the class ID of the component.
	@return						the component pointer matching the ID specified. */
	BaseECSComponent * getComponentInternal(std::vector<std::pair<int, int>>& entityComponents, std::vector<uint8_t> & array, const int & componentID);
	/** Retrieve the components relevant to an ecs system.
	@param	componentTypes		list of component types to retrieve.
	@param	componentFlags		list of flags, designating a component as required or optional. */
	std::vector<std::vector<BaseECSComponent*>> getRelevantComponents(const std::vector<int> & componentTypes, const std::vector<int> & componentFlags);
	/** Find the least common component.
	@param	componentTypes		the component types.
	@param	componentFlags		the component flags. */
	size_t findLeastCommonComponent(const std::vector<int> & componentTypes, const std::vector<int> & componentFlags);


	// Private Attributes
	bool m_finishedLoading = false;
	std::map<int, std::vector<uint8_t>> m_components;
	std::vector<std::pair<int, std::vector<std::pair<int, int>>>*> m_entities;
	std::vector<std::string> m_names;
	Shared_Level m_level;
	WorldState m_state = unloaded;
	std::vector<std::pair<std::shared_ptr<bool>, std::function<void(const WorldState&)>>> m_notifyees;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // WORLD_MODULE_h