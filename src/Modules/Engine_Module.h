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
	/** Construct an engine module. */
	inline Engine_Module() = default;


	// Public Interface Declarations
	/** Initialize the module. */
	inline virtual void initialize(Engine* engine) noexcept { m_engine = engine; };
	/** De-initialize the module. */
	inline virtual void deinitialize() noexcept {}


protected:
	Engine* m_engine = nullptr;
};

#endif // ENGINE_MODULE_H
