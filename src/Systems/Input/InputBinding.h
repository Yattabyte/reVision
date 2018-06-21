#pragma once
#ifndef SYSTEM_INPUT_BINDING_H
#define SYSTEM_INPUT_BINDING_H

#include "Systems\System_Interface.h"
#include "Assets\Asset_Config.h"
#include "Systems\Input\ActionState.h"
#include <string>


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
	 * @param	filename	an optional relative path to a key-bind file to load. Defaults to binds.cfg */
	InputBinding(const std::string & filename = "binds") { Asset_Config::Create(bindings, filename, ActionState::Action_Strings(), false); }


	// Public Methods
	/** Retrieve the key-bindings.
	 * @return	the configuration asset used */
	const Shared_Asset_Config & getBindings() const { return bindings; };
	

private:
	// Private Attributes
	Shared_Asset_Config bindings;
};

#endif // SYSTEM_INPUT_BINDING_H