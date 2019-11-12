#pragma once
#ifndef SYSTEM_INPUT_BINDING_H
#define SYSTEM_INPUT_BINDING_H

#include "Assets/Config.h"
#include "Utilities/ActionState.h"
#include <string>


class Engine;

/** Responsible for loading a particular key-binding configuration. */
class InputBinding {
public:
	// Public (De)Constructors
	/** Destroy the binding (not from disk) */
	~InputBinding();
	/** Construct a key-binding.
	@param	engine		the engine */
	explicit InputBinding(Engine* engine);


	// Public Methods
	/** Loads a preference file from disk.
	@param	filename	the relative path to the bindings file to load */
	void loadFile(const std::string& filename);
	/** Saves the preference file to disk, using the same filename as when loaded. */
	void save();
	/** Retrieve the key-bindings.
	@return	the configuration asset used */
	const Shared_Config& getBindings() const;


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	Shared_Config m_config;
};

#endif // SYSTEM_INPUT_BINDING_H