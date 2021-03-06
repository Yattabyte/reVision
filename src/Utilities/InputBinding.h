#pragma once
#ifndef SYSTEM_INPUT_BINDING_H
#define SYSTEM_INPUT_BINDING_H

#include "Assets/Config.h"
#include <string>


// Forward Declarations
class Engine;

/** Responsible for loading a particular key-binding configuration. */
class InputBinding {
public:
	// Public (De)Constructors
	/** Destroy the binding (not from disk). */
	~InputBinding();
	/** Construct a key-binding.
	@param	engine		reference to the engine to use. */
	explicit InputBinding(Engine& engine) noexcept;
	/** Move an input binding. */
	inline InputBinding(InputBinding&&) noexcept = default;
	/** Copy an input binding. */
	inline InputBinding(const InputBinding&) noexcept = default;


	// Public Methods
	/** Disallow Input binding move assignment. */
	inline InputBinding& operator =(InputBinding&&) noexcept = delete;
	/** Disallow Input binding copy assignment. */
	inline InputBinding& operator =(const InputBinding&) noexcept = delete;
	/** Loads a preference file from disk.
	@param	filename	the relative path to the bindings file to load. */
	void loadFile(const std::string& filename);
	/** Saves the preference file to disk, using the same filename as when loaded. */
	void save();
	/** Retrieve the key-bindings.
	@return	the configuration asset used. */
	Shared_Config getBindings() const noexcept;


private:
	// Private Attributes
	Engine& m_engine;
	Shared_Config m_config;
};

#endif // SYSTEM_INPUT_BINDING_H