#pragma once
#ifndef COMPONENT_H
#define COMPONENT_H

#include "Systems\World\ECS\ECSmessage.h"
#include "Utilities\MappedChar.h"
#include "glm\glm.hpp"
#include <utility>
#include <functional>

using namespace glm;
class ComponentCreator;
class Engine;


/**
 * A base class which is extend-able to create a specific component type.
 * Created by the component factory.
 **/
class Component
{
public:
	/** Returns the name of this component class. */
	virtual const char * getName() const {	return "Component";	}
	/** Sends a command to this component to execute.
	 * @param	command		the string command name
	 * @param	obj			any arguments needed */
	template <typename DATA_TYPE>
	void sendCommand(const char * command, const DATA_TYPE & obj) {
		// Run the command if it exists, and pass it the payload
		if (m_commandMap.find(command))
			m_commandMap[command](ECS_Command(obj));
	}
	/** Returns true if this component is loaded, false otherwise. */
	virtual bool isLoaded() const { return this ? true : false; }
	/** Tests if this object is within the viewing frustum of the camera.
	 * @brief				a test of general visibility (excluding obstruction of other objects).
	 * @param	radius		the radius of the camera
	 * @param	eyePosition	the viewing position of the camera
	 * @return				true if this object is within the viewing range of the camera, false otherwise */
	virtual bool isVisible(const float & radius, const vec3 & eyePosition) const { return true; }
	/** Tests if this object can contain the 3D point specified.
	 * @param	point		the point to test */
	virtual bool containsPoint(const vec3 & point) const { return false; }



protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Component() {};
	/** Constructor. */
	Component() {};


	// Protected Attributes
	MappedChar<function<void(const ECS_Command&)>> m_commandMap;
	friend class ComponentCreator;
};

/**
 * An interface to direct the creation of specific components.
 **/
class ComponentCreator
{
public:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~ComponentCreator(void) {};
	/** Constructor. */
	ComponentCreator() {};


	// Public Methods
	/** Destroy the component.
	 * @param	component	the component to delete */
	void destroy(Component * component) { delete component; };
	/** Creates an component.
	 * @param	engine	pointer to the engine pointer	
	 * @return			the component created */
	virtual Component* create(Engine * engine) { return new Component(); };
};

#endif // COMPONENT_H