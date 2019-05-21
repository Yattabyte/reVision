#pragma once
#ifndef ECS_H
#define ECS_H

#include "Utilities/ECS/ecsComponent.h"
#include "Utilities/ECS/ecsSystem.h"
#include "Utilities/MappedChar.h"
#include <any>
#include <map>
#include <vector>


/** The core class behind the functionality of an ECS framework. */
class ECS {
public:
	// Public (de)Constructors
	~ECS();
	ECS() = default;
	ECS(const ECS& other) = delete;
	void operator=(const ECS& other) = delete;


	// Public Entity Functions
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
	/** Adds a component constructor to the construction map. 
	@param	name				the component name type.
	@param	constructor			the component constructor object. */
	void registerConstructor(const char * name, BaseECSComponentConstructor * constructor);
	/** Construct a component of the type provided, using the parameters specified.
	@param	typeName			the component name type.
	@param	parameters			the construction parameters (arguments). */
	const Component_and_ID constructComponent(const char * typeName, const std::vector<std::any> & parameters);

	// Public System Functions
	/** Update the components of all systems provided.
	@param	systems				the systems to update.
	@param	deltaTime			the delta time. */
	void updateSystems(ECSSystemList & systems, const float & deltaTime);
	/** Update the components of a single system. 
	@param	system				the system to update.
	@param	deltaTime			the delta time. */
	void updateSystem(BaseECSSystem * system, const float & deltaTime);
	/** Purge the ECS, unloading all entities and components. */
	void purge();


private:
	// Private Functions
	inline std::pair< uint32_t, std::vector<std::pair<uint32_t, uint32_t> > >* handleToRawType(const EntityHandle & handle) {
		return (std::pair< uint32_t, std::vector<std::pair<uint32_t, uint32_t> > >*)handle;
	}
	inline uint32_t handleToEntityIndex(const EntityHandle & handle) {
		return handleToRawType(handle)->first;
	}
	inline std::vector<std::pair<uint32_t, uint32_t> >& handleToEntity(const EntityHandle & handle) {
		return handleToRawType(handle)->second;
	}
	void deleteComponent(const uint32_t & componentID, const uint32_t & index);
	void addComponentInternal(EntityHandle handle, std::vector<std::pair<uint32_t, uint32_t>> & entity, const uint32_t & componentID, BaseECSComponent * component);
	bool removeComponentInternal(EntityHandle handle, const uint32_t & componentID);
	BaseECSComponent * getComponentInternal(std::vector<std::pair<uint32_t, uint32_t>>& entityComponents, std::vector<uint8_t> & array, const uint32_t & componentID);
	void updateSystemWithMultipleComponents(BaseECSSystem * system, const float & deltaTime, const std::vector<uint32_t> & componentTypes);
	size_t findLeastCommonComponent(const std::vector<uint32_t> & componentTypes, const std::vector<uint32_t> & componentFlags);


	// Private attributes
	std::map<uint32_t, std::vector<uint8_t>> m_components;
	std::vector < std::pair< uint32_t, std::vector<std::pair<uint32_t, uint32_t> > >* > m_entities;
	MappedChar<BaseECSComponentConstructor*> m_constructorMap;
};

#endif // ECS_H
