#pragma once
#ifndef ECS_H
#define ECS_H

#include "ECS\Components\Component.h"
#include "ECS\Entity.h"
#include "ECS\ECSmessage.h"
#include "Systems\World\Visibility_Token.h"
#include "Utilities\MappedChar.h"
#include <deque>
#include <shared_mutex>


class Engine;

/**
 * A utility that handles the creation and storage of all level components and entities.
 **/
class ECS {
public:
	// (de)Constructors
	/** Destroy the ECS. */
	~ECS();
	/** Construct the ECS.
	 * @param	engine	pointer to the engine pointer */
	ECS(Engine * engine) : m_engine(engine) {}

	
	// Public Methods	
	/** Creates an entity with the supplied components
	 * @param	args			a comma delineated list of component pointers to add to the entity
	 * @return					the newely created entity */	
	template <typename... Args>
	inline Entity * createEntity(Args&&... ax) {
		const unsigned int count	= sizeof...(Args);
		const char * types[]		= { ax->GetName()... };
		Component * components[]	= { ax... };
		Entity * entity				= new Entity(count, types, components);

		std::unique_lock<std::shared_mutex> write_lock(m_dataLock);
		m_levelEntities.push_back(entity);
		return entity;
	}
	/** Deletes the entity provided.
  	 * @param	entity				the entity to delete */
	void deleteEntity(Entity * entity);
	/** Creates a component of the supplied type and returns its pointer.
	 * @param	<Component_Type>	the class of component to create
	 * @param	args				any optional arguments to send to the component's constructor
	 * @return						the newely created component */	
	template <typename Component_Type, typename... Args>
	Component_Type * createComponent(Args&&... ax) {
		const char * componentName = Component_Type::GetName();
		Component_Type * component = new Component_Type(m_engine, std::forward<Args>(ax)...);
		std::unique_lock<std::shared_mutex> write_lock(m_dataLock);

		m_levelComponents.insert(componentName);
		if (m_freeSpots.find(componentName) && m_freeSpots[componentName].size()) {
			m_levelComponents[componentName][m_freeSpots[componentName].front()] = component;
			m_freeSpots[componentName].pop_front();
		}
		else
			m_levelComponents[componentName].push_back(component);

		return component;
	}
	/** Deletes the component provided.
  	 * @param	component			the component to delete */
	void deleteComponent(Component * component);	
	/** Retrieves reference to the level components std::vector-map
	* @return					the entire level component std::vector-map */
	VectorMap<Component*> & getComponents();
	/** Retrieves an array of components that match the category specified.
	 * @brief					Guaranteed to return at least a zero-length std::vector. Types that don't exist are created.
	 * @param	type			the type-name of the component list to retrieve
	 * @return					the list of components that match the type provided */
	const std::vector<Component*> & getComponentsByType(const char * type);
	/** Retrieve and down-cast an array of components that match the category specified.
	 * @brief					Guaranteed to return at least a zero-length std::vector. Types that don't exist are created.
	 * @param	type			the name of the component type to retrieve
	 * @param	<T>				the class-type to cast the components to */
	template <typename T>
	const std::vector<T*> getSpecificComponents(const char * type) {
		// Want to return a copy because this data would need to be locked until done being used at its target otherwise.
		std::shared_lock<std::shared_mutex> read_lock(getDataLock());
		return *(std::vector<T*>*)(&getComponentsByType(type));
	}
	/** Removes all components from the system. */
	void flush();	
	/** Checks the map to see if it has any entries of a specific type.
	 * @param	key				the type to check
	 * @return					true if it finds the key in the map, false otherwise */
	bool find(const char * key) const;
	/** Returns the data lock for the system. 
	 * @return					the std::mutex for this class */
	std::shared_mutex & getDataLock();


private:
	// Private Attributes
	bool m_Initialized;
	VectorMap<Component*> m_levelComponents;
	std::vector<Entity*> m_levelEntities;
	MappedChar<std::deque<unsigned int>> m_freeSpots;
	mutable std::shared_mutex m_dataLock;
	Engine * m_engine;
};

#endif // ECS_H