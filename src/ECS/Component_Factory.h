#pragma once
#ifndef COMPONENT_FACTORY_H
#define COMPONENT_FACTORY_H

#include "ECS\Components\Component.h"
#include "ECS\ECSmessage.h"
#include "Systems\World\Visibility_Token.h"
#include "Utilities\MappedChar.h"
#include <deque>
#include <shared_mutex>


class Engine;

/**
 * A utility that handles the creation and storage of all level components
 **/
class Component_Factory {
public:
	// (de)Constructors
	/** Destroy the factory. */
	~Component_Factory();
	/** Construct the factory. */
	Component_Factory();

	
	// Public Methods
	/** Initialize  the component factory.
	 * @param	engine	pointer to the engine pointer */
	void initialize(Engine * engine);
	/** Creates a component of the supplied type and returns its handle.
	 * @param	type			the type of component to create
	 * @param	args			any optional arguments to send to the component's constructor
	 * @return					the newely created component */	
	template <typename Component_Type, typename... Args>
	Component_Type * createComponent(const char * type, Args&&... ax) {
		Component_Type * component = new Component_Type(m_engine, std::forward<Args>(ax)...);
		std::unique_lock<std::shared_mutex> write_lock(m_dataLock);

		m_levelComponents.insert(type);
		if (m_freeSpots.find(type) && m_freeSpots[type].size()) {
			m_levelComponents[type][m_freeSpots[type].front()] = component;
			m_freeSpots[type].pop_front();
		}
		else
			m_levelComponents[type].push_back(component);

		return component;

	}
	/** Deletes the component provided.
  	 * @param	component		the component to delete */
	void deleteComponent(Component * component);	
	/** Retrieves reference to the level components std::vector-map
	* @return						the entire level component std::vector-map */
	VectorMap<Component*> & getComponents();
	/** Retrieves an array of components that match the category specified.
	 * @brief					Guaranteed to return at least a zero-length std::vector. Types that don't exist are created.
	 * @param	type			the type-name of the component list to retrieve
	 * @return					the list of components that match the type provided */
	const std::vector<Component*> & getComponentsByType(const char * type);
	/** Removes all components from the system. */
	void flush();	
	/** Checks the map to see if it has any entries of a specific type.
	 * @param	key				the type to check
	 * @return					true if it finds the key in the map, false otherwise */
	bool find(const char * key) const;
	/** Returns the data lock for the system. 
	 * @return					the std::mutex for this factory */
	std::shared_mutex & getDataLock();


private:
	// Private Attributes
	bool m_Initialized;
	VectorMap<Component*> m_levelComponents;	
	MappedChar<std::deque<unsigned int>> m_freeSpots;
	mutable std::shared_mutex m_dataLock;
	Engine *m_engine;
};

#endif // COMPONENT_FACTORY_H