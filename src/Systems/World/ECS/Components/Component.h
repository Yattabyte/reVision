#pragma once
#ifndef COMPONENT
#define COMPONENT
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECS\ECSmessage.h"
#include <utility>

class ECSmessenger;
class ComponentCreator;
class EnginePackage;


/**
 * A base class which is extend-able to create a specific component type.
 * Created by the component factory.
 **/
class DT_ENGINE_API Component
{
public:
	/** Have this component accept a message.
	 * @brief				a handy way to interface with components.
	 * @param	message		the message to send to this component */
	virtual void receiveMessage(const ECSmessage & message);


protected:
	// (de)Constructors
	/** Virtual Destructor. */
	virtual ~Component() {};
	/** Constructor. 
	 * @param	id			handle for this component (into component factory)
	 * @param	pid			handle for the parent entity (into entity factory) */
	Component(const ECShandle & id, const ECShandle & pid) : m_ID(id), m_parentID(pid) {};
	/** Tests a message to determine if it originated from this component.
	 * @param	message		the message to test
	 * @return				true if this component is the sender, false otherwise */
	bool compareMSGSender(const ECSmessage & message);


	// Protected Attributes
	ECShandle m_ID, m_parentID;
	ECSmessenger *m_ECSmessenger;
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
	ComponentCreator(ECSmessenger * ecsMessenger) : m_ECSmessenger(ecsMessenger) {};


	// Public Methods
	/** Destroy the component.
	 * @param	component	the component to delete */
	void Destroy(Component * component) { delete component; };
	/** Creates an component.
	 * @param	id				the handle identifier for the component
	 * @param	pid				the handle identifier for the parent entity
	 * @param	enginePackage	pointer to the engine package	
	 * @return					the component created */
	virtual Component* Create(const ECShandle & id, const ECShandle & pid, EnginePackage * enginePackage) { return new Component(id, pid); };
	

	// Public Attributes
	ECSmessenger *m_ECSmessenger;
};

#endif // COMPONENT