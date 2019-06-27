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
	/** Unload the current world. */
	void unloadWorld();
	/** Registers a notification function to be called when the world state changes.
	@param	alive		a shared pointer indicating whether the caller is still alive or not.
	@param	notifier	function to be called on state change. */
	void addLevelListener(const std::shared_ptr<bool> & alive, const std::function<void(const WorldState&)> & func);
	/** Construct an entity from the array of components and IDS*/
	EntityHandle makeEntity(BaseECSComponent ** components, const uint32_t * componentIDS, const size_t & numComponents);
	/** Construct an entity from the array of component references.
	@note Variadic
	@param	args	all components to use for this entity. */
	template <class...Args>
	inline EntityHandle makeEntity(Args&...args) {
		BaseECSComponent * components[] = { &args... };
		const uint32_t componentIDS[] = { Args::ID... };
		return makeEntity(components, componentIDS, sizeof...(Args));
	}
	/** Construct an entity from the array of component pointers.
	@note Variadic
	@param	args	all components to use for this entity. */
	template <class...Args>
	inline EntityHandle makeEntity(Args*...args) {
		BaseECSComponent * components[] = { args... };
		const uint32_t componentIDS[] = { Args::ID... };
		return makeEntity(components, componentIDS, sizeof...(Args));
	}
	/** Remove an entity.
	@param	handle	the entity to remove. */
	void removeEntity(const EntityHandle & handle);
	// Public BaseECSComponent Functions
	/** Adds a component to an entity.
	@param	entity				the entity to add the component to.
	@param	component			the component being added. */
	template <typename BaseECSComponent>
	inline void addComponent(const EntityHandle & entity, BaseECSComponent * component) {
		addComponentInternal(entity, handleToEntity(entity), BaseECSComponent::ID, component);
	}
	/** Removes a component from an entity.
	@param	entity				the entity to remove from.
	@param	<BaseECSComponent>	the category of component being removed.
	@return						true on successful removal, false otherwise. */
	template <typename BaseECSComponent>
	inline bool removeComponent(const EntityHandle & entity) {
		return removeComponentInternal(entity, BaseECSComponent::ID);
	}
	/** Retrieve a component.
	@param	entity				the entity to retrieve from.
	@param	<BaseECSComponent>	the category of component being retrieved. */
	template <typename BaseECSComponent>
	inline BaseECSComponent * getComponent(const EntityHandle & entity) {
		return (BaseECSComponent*)getComponentInternal(handleToEntity(entity), m_components[BaseECSComponent::ID], BaseECSComponent::ID);
	}
	/** Add support for a specific component type at level-creation-time.
	@param	name				the component class name.
	@param	func				the component creation function. */
	void addComponentType(const char * name, const std::function<std::pair<uint32_t, BaseECSComponent*>(const ParamList &)> & func);
	/** Remove support for a specific component type at level-creation-time. 
	@param	name				the component class name. */
	void removeComponentType(const char * name);
	/** Add a callback method for when a component of the given type is created.
	@param	ID					the component class ID. 
	@param	alive				a shared pointer indicating whether the caller is still alive or not.
	@param	func				the function to callback when a component of class name is created. */
	void addNotifyOnComponentType(const uint32_t & ID, const std::shared_ptr<bool> & alive, const std::function<void(BaseECSComponent *)> & func);
	/** Update the components of all systems provided.
	@param	systems				the systems to update.
	@param	deltaTime			the delta time. */
	void updateSystems(ECSSystemList & systems, const float & deltaTime);
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	void updateSystem(BaseECSSystem * system, const float & deltaTime);
	/***/
	void updateSystem(const float & deltaTime, const std::vector<uint32_t> & types, const std::vector<uint32_t> & flags, const std::function<void(const float &, const std::vector<std::vector<BaseECSComponent*>>&)> &func);


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
	inline std::pair< uint32_t, std::vector<std::pair<uint32_t, uint32_t> > >* handleToRawType(const EntityHandle & handle) {
		return (std::pair< uint32_t, std::vector<std::pair<uint32_t, uint32_t> > >*)handle;
	}
	/** Convert an entity handle to its raw index. 
	@param	handle				the entity handle to process.
	@return						raw entity index. */
	inline uint32_t handleToEntityIndex(const EntityHandle & handle) {
		return handleToRawType(handle)->first;
	}
	/** Convert an entity handle to its raw data.
	@param	handle				the entity handle to process.
	@return						raw entity data. */
	inline std::vector<std::pair<uint32_t, uint32_t> >& handleToEntity(const EntityHandle & handle) {
		return handleToRawType(handle)->second;
	}
	/** Delete a component matching the category ID supplied, at the given index. 
	@param	componentID			the component class/category ID.
	@param	index				the component index to delete. */
	void deleteComponent(const uint32_t & componentID, const uint32_t & index);
	/** Adds a component to the entity specified.
	@param	handle				the entity handle, to add the component to.
	@param	entity				the specific entity data array.
	@param	componentID			the class ID of the component.
	@param	component			the specific component to add to the entity. */
	void addComponentInternal(EntityHandle handle, std::vector<std::pair<uint32_t, uint32_t>> & entity, const uint32_t & componentID, BaseECSComponent * component);
	/** Remove a component from the entity specified.
	@param	handle				the entity handle, to remove the component from.
	@param	componentID			the class ID of the component. 
	@return						true on remove success, false otherwise. */
	bool removeComponentInternal(EntityHandle handle, const uint32_t & componentID);
	/** Retrieve the component from an entity matching the class specified.
	@param	entityComponents	the array of entity component IDS.
	@param	array				the array of component data.
	@param	componentID			the class ID of the component.
	@return						the component pointer matching the ID specified. */
	BaseECSComponent * getComponentInternal(std::vector<std::pair<uint32_t, uint32_t>>& entityComponents, std::vector<uint8_t> & array, const uint32_t & componentID);
	/***/
	std::vector<std::vector<BaseECSComponent*>> getRelevantComponents(const std::vector<uint32_t> & componentTypes, const std::vector<uint32_t> & componentFlags);
	/** Find the least common component.
	@param	componentTypes		the component types.
	@param	componentFlags		the component flags. */
	size_t findLeastCommonComponent(const std::vector<uint32_t> & componentTypes, const std::vector<uint32_t> & componentFlags);


	// Private Attributes
	bool m_finishedLoading = false;
	std::map<uint32_t, std::vector<uint8_t>> m_components;
	std::vector<std::pair<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>>*> m_entities;
	MappedChar<std::function<std::pair<uint32_t,BaseECSComponent*>(const ParamList &)>> m_constructorMap;
	std::map<uint32_t, std::vector<std::pair<std::shared_ptr<bool>, std::function<void(BaseECSComponent*)>>>> m_constructionNotifyees;
	Shared_Level m_level;
	WorldState m_state = unloaded;
	std::vector<std::pair<std::shared_ptr<bool>, std::function<void(const WorldState&)>>> m_notifyees;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // WORLD_MODULE_h