#pragma once
#ifndef ECS_H
#define ECS_H

#include "ECS\Components\ecsComponent.h"
#include "ECS\Systems\ecsSystem.h"
#include <map>
#include <vector>


/** The core class behind the functionality of an ECS framework. */
class ECS {
public:
	// Public (de)Constructors
	ECS() {}
	~ECS();
	ECS(const ECS& other) { (void)other; }
	void operator=(const ECS& other) { (void)other; }


	// Public Entity Functions
	/** Construct an entity from the array of components and IDS*/
	EntityHandle makeEntity(BaseECSComponent ** components, const unsigned int * componentIDS, const size_t & numComponents);
	/** Construct an entity from the array of component references.
	@note Variadic
	@param	args	all components to use for this entity. */
	template <class...Args>
	inline EntityHandle makeEntity(Args&...args) {
		BaseECSComponent * components[] = { &args... };
		unsigned int componentIDS[] = { Args::ID... };
		return makeEntity(components, componentIDS, sizeof...(Args));
	}
	/** Construct an entity from the array of component pointers.
	@note Variadic
	@param	args	all components to use for this entity. */
	template <class...Args>
	inline EntityHandle makeEntity(Args*...args) {
		BaseECSComponent * components[] = { args... };
		unsigned int componentIDS[] = { Args::ID... };
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
		return (BaseECSComponent*)getComponentInternal(handleToEntity(entity), components[BaseECSComponent::ID], BaseECSComponent::ID);
	}


	// Public System Functions
	/** Update the components of all systems provided.
	@param	systems				the systems to update.
	@param	deltaTime			the delta time. */
	void updateSystems(ECSSystemList & systems, const float & deltaTime);


private:
	// Private Functions
	inline std::pair< unsigned int, std::vector<std::pair<unsigned int, unsigned int> > >* handleToRawType(const EntityHandle & handle) {
		return (std::pair< unsigned int, std::vector<std::pair<unsigned int, unsigned int> > >*)handle;
	}
	inline unsigned int handleToEntityIndex(const EntityHandle & handle) {
		return handleToRawType(handle)->first;
	}
	inline std::vector<std::pair<unsigned int, unsigned int> >& handleToEntity(const EntityHandle & handle) {
		return handleToRawType(handle)->second;
	}
	void deleteComponent(const unsigned int & componentID, const unsigned int & index);
	void addComponentInternal(EntityHandle handle, std::vector<std::pair<unsigned int, unsigned int>> & entity, const unsigned int & componentID, BaseECSComponent * component);
	bool removeComponentInternal(EntityHandle handle, const unsigned int & componentID);
	BaseECSComponent * getComponentInternal(std::vector<std::pair<unsigned int, unsigned int>>& entityComponents, std::vector<unsigned int> & array, const unsigned int & componentID);
	void updateSystemWithMultipleComponents(ECSSystemList & systems, const unsigned int & index, const float & deltaTime, const std::vector<unsigned int> & componentTypes);
	unsigned int findLeastCommonComponent(const std::vector<unsigned int> & componentTypes, const std::vector<unsigned int> & componentFlags);


	// Private attributes
	std::map<unsigned int, std::vector<unsigned int>> components;
	std::vector < std::pair< unsigned int, std::vector<std::pair<unsigned int, unsigned int> > >* > entities;	
};

#endif // ECS_H
