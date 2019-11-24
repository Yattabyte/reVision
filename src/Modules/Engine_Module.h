#pragma once
#ifndef ENGINE_MODULE_H
#define ENGINE_MODULE_H


class Engine;

/** An interface for engine modules to implement. */
class Engine_Module {
public:
	// Public (De)Constructors
	/** Destroy this engine module. */
	inline virtual ~Engine_Module() = default;
	/** Construct an engine module.
	@param	engine		the currently active engine. */
	inline explicit Engine_Module(Engine& engine) : m_engine(engine) {}


	// Public Interface Declarations
	/** Initialize the module. */
	inline virtual void initialize() noexcept {};
	/** De-initialize the module. */
	inline virtual void deinitialize() noexcept {}


protected:
	Engine& m_engine;
};

#endif // ENGINE_MODULE_H
