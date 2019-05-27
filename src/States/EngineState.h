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
	/** Handles the input for the engine.
	@param	actionState		the engine action state.
	@return					a potentially new engine state if the input dictates a change of state, nullptr otherwise. */
	inline virtual EngineState * handleInput(ActionState & actionState) { return nullptr; }
	/** Peforms additional tick operations.
	@param	deltaTime		amount of time that passed since last tick. */
	inline virtual void handleTick(const float & deltaTime) {}


protected:
	// Protected
	Engine * m_engine = nullptr;
};

#endif // ENGINESTATE_H
