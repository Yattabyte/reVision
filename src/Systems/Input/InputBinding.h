#pragma once
#ifndef SYSTEM_INPUT_BINDING_H
#define SYSTEM_INPUT_BINDING_H

#include "Systems\System_Interface.h"
#include "Assets\Asset_Config.h"
#include "Systems\Input\ActionState.h"
#include <string>

class Engine;


/**
 * Responsible for loading a particular key-binding configuration
 **/
class InputBinding
{
public: 
	// (de)Constructors
	/** Destroy the binding (not from disk) */
	~InputBinding() {}
	/** Construct a key-binding.
	 * @param	engine		the engine
	 * @param	filename	a relative path to a key-bind file to load */
	InputBinding(Engine * engine, const std::string & filename);


	// Public Methods
	/** Retrieve the key-bindings.
	 * @return	the configuration asset used */
	const Shared_Asset_Config & getBindings() const;
	

private:
	// Private Attributes
	Shared_Asset_Config bindings;
};

#endif // SYSTEM_INPUT_BINDING_H