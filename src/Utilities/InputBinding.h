#pragma once
#ifndef SYSTEM_INPUT_BINDING_H
#define SYSTEM_INPUT_BINDING_H

#include "Assets\Asset_Config.h"
#include "Utilities\ActionState.h"
#include <string>


class Engine;

/** Responsible for loading a particular key-binding configuration. */
class InputBinding
{
public: 
	// (de)Constructors
	/** Destroy the binding (not from disk) */
	~InputBinding() = default;
	/** Construct a key-binding.
	@param	engine		the engine */
	InputBinding(Engine * engine);


	// Public Methods
	/** Loads a preference file from disk.
	@param	filename	the relative path to the bindings file to load */
	void loadFile(const std::string & filename);
	/** Retrieve the key-bindings.
	@return	the configuration asset used */
	const Shared_Config & getBindings() const;
	

private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Config m_config;
};

#endif // SYSTEM_INPUT_BINDING_H