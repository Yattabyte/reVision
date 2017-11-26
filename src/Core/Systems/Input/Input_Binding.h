/*
	Input_Binding

	- Loads a particular key/controller/peripheral binding configuration
*/



#pragma once
#ifndef SYSTEM_INPUT_BINDING
#define SYSTEM_INPUT_BINDING
#ifdef	ENGINE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Systems\System_Interface.h"
#include "Assets\Asset_Config.h"
#include "Utilities\Action_State.h"
#include <string>

class DELTA_CORE_API System_Input_Binding
{
public: 
	~System_Input_Binding() {}
	System_Input_Binding(const std::string &filename = "binds") { Asset_Manager::load_asset(bindings, filename, ACTION_STRINGS, false); }
	const Shared_Asset_Config &getBindings() const { return bindings; };
	
private:
	Shared_Asset_Config bindings;
};

#endif // SYSTEM_INPUT_BINDING