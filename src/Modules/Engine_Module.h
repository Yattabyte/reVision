#pragma once
#ifndef ENGINE_MODULE_H
#define ENGINE_MODULE_H


class Engine;

/** An interface for engine modules to implement. */
class Engine_Module {
public:
	// Public (de)Constructors
	/** Destroy this engine module. */
	inline ~Engine_Module() = default;
	/** Construct an engine module. */
	inline Engine_Module() = default;

	
	// Public Interface Declarations
	/** Initialize the module. */
	inline virtual void initialize(Engine * engine) { m_engine = engine; };
	/** Tick the ui by a frame.
	@param	deltaTime	the amount of time passed since last frame. */
	inline virtual void frameTick(const float &) {}


protected:
	Engine * m_engine = nullptr;
};

#endif // ENGINE_MODULE_H