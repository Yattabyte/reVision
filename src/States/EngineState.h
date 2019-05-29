#pragma once
#ifndef ENGINESTATE_H
#define ENGINESTATE_H

#include "Utilities/ActionState.h"


class Engine;

/** Represents the interface for the engine, in a particular controlable state. */
class EngineState {
public:
	// Public (de)Constructors
	/** Destroy this state. */
	inline virtual ~EngineState() = default;
	/** Construct this state. */
	inline EngineState(Engine * engine) : m_engine(engine) {}


	// Public Interface Declaration
	/** Peforms additional tick operations.
	@param	deltaTime		amount of time that passed since last tick. */
	inline virtual void handleTick(const float & deltaTime) {}


protected:
	// Protected
	Engine * m_engine = nullptr;
};

#endif // ENGINESTATE_H
