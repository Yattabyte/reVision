#pragma once
#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "States/EngineState.h"
#include "States/Game/ECS/Player3D_C.h"
#include "States/Game/ECS/PlayerFreeLook_S.h"
#include "Engine.h"


class GameState : public EngineState {
public:
	// Public (de)Constructors
	inline ~GameState() = default;
	inline GameState(Engine * engine) : EngineState(engine) {
		m_freeLookSystem = new PlayerFreeLook_System(engine);
		m_engine->registerECSConstructor("Player3D_Component", new Player3D_Constructor());
		m_engine->getModule_World().loadWorld("A.map");
		glfwSetInputMode(m_engine->getRenderingContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	 

	// Public Interface Implementation
	inline virtual EngineState * handleInput(ActionState & actionState) override {
		actionState[ActionState::LOOK_X] = actionState[ActionState::MOUSE_X];
		actionState[ActionState::LOOK_Y] = actionState[ActionState::MOUSE_Y];
		glfwSetCursorPos(m_engine->getRenderingContext(), 0, 0);
		return nullptr;
	}
	inline virtual void handleTick(const float & deltaTime) override {
		if (m_freeLookSystem->isValid()) {
			if (m_engine->getModule_World().checkIfLoaded())
				m_engine->getModule_Physics().frameTick(deltaTime);
			m_engine->getECS().updateSystem(m_freeLookSystem, deltaTime);
			m_engine->getModule_Graphics().setActiveCamera(0);
			m_engine->getModule_Graphics().frameTick(deltaTime);
			m_engine->getModule_PostProcess().frameTick(deltaTime);
		}
	}


protected:
	// Protected Attributes
	BaseECSSystem * m_freeLookSystem;
};

#endif // GAMESTATE_H