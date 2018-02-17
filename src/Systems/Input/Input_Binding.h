#pragma once
#ifndef SYSTEM_INPUT_BINDING
#define SYSTEM_INPUT_BINDING
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\System_Interface.h"
#include "Assets\Asset_Config.h"
#include "Systems\Input\Action_State.h"
#include <string>


/**
 * Responsible for loading a particular key-binding configuration
 **/
class DT_ENGINE_API Input_Binding
{
public: 
	// (de)Constructors
	/** Destroy the binding (not from disk) */
	~Input_Binding() {}
	/** Construct a key-binding.
	 * @param	filename	an optional relative path to a key-bind file to load. Defaults to binds.cfg */
	Input_Binding(const std::string & filename = "binds") { Asset_Loader::load_asset(bindings, filename, Action_State::Action_Strings(), false); }


	// Public Methods
	/** Retrieve the key-bindings.
	 * @return	the configuration asset used */
	const Shared_Asset_Config & getBindings() const { return bindings; };
	

private:
	// Private Attributes
	Shared_Asset_Config bindings;
};

#endif // SYSTEM_INPUT_BINDING