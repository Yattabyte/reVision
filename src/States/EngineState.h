#pragma once
#ifndef ENGINESTATE_H
#define ENGINESTATE_H

#include "Utilities/ActionState.h"


class Engine;

/***/
class EngineState {
public:
	// Public (de)Constructors
	/** Destroy this state. */
	inline virtual ~EngineState() = default;
	/** Construct this state. */
	inline EngineState(Engine * engine) : m_engine(engine) {}


	// Public Interface Declaration
	/***/
	inline virtual EngineState * handleInput(ActionState & actionState) { return nullptr; }
	/***/
	inline virtual void handleTick(const float & deltaTime) {}


protected:
	// Protected
	Engine * m_engine = nullptr;
};

#endif // ENGINESTATE_H
