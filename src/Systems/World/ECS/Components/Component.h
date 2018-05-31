#pragma once
#ifndef COMPONENT
#define COMPONENT
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECS\ECSmessage.h"
#include "Utilities\MappedChar.h"
#include "glm\glm.hpp"
#include <utility>
#include <functional>

using namespace glm;
class ComponentCreator;
class EnginePackage;


/**
 * A base class which is extend-able to create a specific component type.
 * Created by the component factory.
 **/
class DT_ENGINE_API Component
{
public:
	/** Returns the name of this component class. */
	virtual const char * getName() const {	return "Component";	}
	template <typename DATA_TYPE>
	void sendCommand(const char * command, const DATA_TYPE & obj) {
		// Run the command if it exists, and pass it the payload
		if (m_commandMap.find(command))
			m_commandMap[command](ECS_Command(obj));
	}
	/** Tests if this object is within the viewing frustum of the camera.
	 * @brief				a test of general visibility (excluding obstruction of other objects).
	 * @param	radius		the radius of the camera
	 * @param	eyePosition	the viewing position of the camera
	 * @return				true if this object is within the viewing range of the camera, false otherwise */
	virtual bool isVisible(const float & radius, const vec3 & eyePosition) const { return true; }
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
class DT_ENGINE_API ComponentCreator
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
	 * @param	enginePackage	pointer to the engine package	
	 * @return					the component created */
	virtual Component* create(EnginePackage * enginePackage) { return new Component(); };
};

#endif // COMPONENT