#pragma once
#ifndef ENGINESTATE_H
#define ENGINESTATE_H

#include "Utilities/ActionState.h"


class Engine;

class EngineState {
public:
	// Public (de)Constructors
	inline virtual ~EngineState() = default;
	inline EngineState(Engine * engine) : m_engine(engine) {}


	// Public Interface Declaration
	/***/
	virtual EngineState * handleInput(ActionState & actionState) { return nullptr; } 
	/***/
	virtual void handleTick(const float & deltaTime) {}


protected:
	// Protected
	Engine * m_engine = nullptr;
};

#endif // ENGINESTATE_H
