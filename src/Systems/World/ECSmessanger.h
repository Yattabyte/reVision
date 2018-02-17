#pragma once
#ifndef ECSMESSANGER
#define ECSMESSANGER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECSmessage.h"
#include "Systems\World\ECSdefines.h"
#include <map>
#include <vector>

class Entity_Factory;
class Component_Factory;


/**
 * A utility that handles the deliver of messages between entities and components.
 **/
class DT_ENGINE_API ECSmessanger {
public:
	// (de)Constructors
	/** Destroy the messanger. */
	~ECSmessanger();
	/** Construct the messanger.
	 * @param	entityFactory		pointer to the entity factory
	 * @param	componentFactory	pointer to the component factory */
	ECSmessanger(Entity_Factory *entityFactory, Component_Factory *componentFactory);


	// Public Methods
	/** Propagates a message towards an entity matching the handle provided
	 * @param	message		the message to send
	 * @param	target		the handle of the desired entity */
	void SendMessage_ToEntity(const ECSmessage &message, const ECShandle &target);
	/** Propagates a message towards a component matching the handle provided
	 * @param	message		the message to send
	 * @param	target		the handle of the desired component */
	void SendMessage_ToComponent(const ECSmessage &message, const ECShandle &target);
	/** Propagates a message towards all components matching the handles provided
	 * @param	message		the message to send
	 * @param	targets		a map of component handles */
	void SendMessage_ToComponents(const ECSmessage &message, const ECShandle_map &targets);


private:
	// Private Attributes
	Entity_Factory *m_entityFactory;
	Component_Factory *m_componentFactory;
};

#endif // ECSMESSANGER
