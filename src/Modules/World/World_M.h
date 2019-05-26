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
	// Public (de)Constructors
	/***/
	~World_Module();
	/***/
	inline World_Module() = default;


	// Public Interface Implementation
	/** Initialize the module. */
	virtual void initialize(Engine * engine) override;
	/** Tick the world by a frame.
	@param	deltaTime	the amount of time passed since last frame */
	virtual void frameTick(const float & deltaTime) override;


	// Public Methods
	/** Loads the world.
	@param	mapName		the name of the map to load. */
	void loadWorld(const std::string & mapName);
	/***/
	void unloadWorld();
	/** Registers a notification flag to be updated when level loaded.
	@param	notifier	flag to be set when level loaded*/
	void addLevelListener(bool * notifier);
	/** Checks whether the level has finished loading. 
	@return				true if level sufficiently loaded, false otherwise. */
	bool checkIfLoaded();
	/** Construct an entity from the array of components and IDS*/
	EntityHandle makeEntity(BaseECSComponent ** components, const uint32_t * componentIDS, const size_t & numComponents);
	/** Construct an entity from the array of component references.
	@note Variadic
	@param	args	all components to use for this entity. */
	template <class...Args>
	inline EntityHandle makeEntity(Args&...args) {
		BaseECSComponent * components[] = { &args... };
		uint32_t componentIDS[] = { Args::ID... };
		return makeEntity(components, componentIDS, sizeof...(Args));
	}
	/** Construct an entity from the array of component pointers.
	@note Variadic
	@param	args	all components to use for this entity. */
	template <class...Args>
	inline EntityHandle makeEntity(Args*...args) {
		BaseECSComponent * components[] = { args... };
		uint32_t componentIDS[] = { Args::ID... };
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
	/***/
	void addComponentType(const char * name, const std::function<std::pair<uint32_t, BaseECSComponent*>(const ParamList &)> & func);
	/***/
	void removeComponentType(const char * name);
	/** Update the components of all systems provided.
	@param	systems				the systems to update.
	@param	deltaTime			the delta time. */
	void updateSystems(ECSSystemList & systems, const float & deltaTime);
	/** Update the components of a single system.
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	void updateSystem(BaseECSSystem * system, const float & deltaTime);
	/** Purge the ECS, unloading all entities and components. */


private:
	// Private Methods
	/** Process the level asset, generating components and entities. */
	void processLevel();
	/***/
	inline std::pair< uint32_t, std::vector<std::pair<uint32_t, uint32_t> > >* handleToRawType(const EntityHandle & handle) {
		return (std::pair< uint32_t, std::vector<std::pair<uint32_t, uint32_t> > >*)handle;
	}
	/***/
	inline uint32_t handleToEntityIndex(const EntityHandle & handle) {
		return handleToRawType(handle)->first;
	}
	/***/
	inline std::vector<std::pair<uint32_t, uint32_t> >& handleToEntity(const EntityHandle & handle) {
		return handleToRawType(handle)->second;
	}
	/***/
	void deleteComponent(const uint32_t & componentID, const uint32_t & index);
	/***/
	void addComponentInternal(EntityHandle handle, std::vector<std::pair<uint32_t, uint32_t>> & entity, const uint32_t & componentID, BaseECSComponent * component);
	/***/
	bool removeComponentInternal(EntityHandle handle, const uint32_t & componentID);
	/***/
	BaseECSComponent * getComponentInternal(std::vector<std::pair<uint32_t, uint32_t>>& entityComponents, std::vector<uint8_t> & array, const uint32_t & componentID);
	/***/
	void updateSystemWithMultipleComponents(BaseECSSystem * system, const float & deltaTime, const std::vector<uint32_t> & componentTypes);
	/***/
	size_t findLeastCommonComponent(const std::vector<uint32_t> & componentTypes, const std::vector<uint32_t> & componentFlags);


	// Private Attributes
	bool m_finishedLoading = false;
	std::map<uint32_t, std::vector<uint8_t>> m_components;
	std::vector<std::pair<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>>*> m_entities;
	MappedChar<std::function<std::pair<uint32_t,BaseECSComponent*>(const ParamList &)>> m_constructorMap;
	Shared_Level m_level;
	std::vector<bool*> m_notifyees;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // WORLD_MODULE_h