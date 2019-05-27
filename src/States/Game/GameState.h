#pragma once
#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "States/EngineState.h"
#include "States/Game/ECS/components.h"
#include "States/Game/ECS/PlayerFreeLook_S.h"
#include "Engine.h"


/** Represents an engine state for the "game". */
class GameState : public EngineState {
public:
	// Public (de)Constructors
	/** Destroy the game state. */
	inline ~GameState() {
		m_engine->getModule_World().removeComponentType("Player3D_Component");
	}
	/** Construct a game state. */
	inline GameState(Engine * engine) : EngineState(engine) {
		m_freeLookSystem = new PlayerFreeLook_System(engine);
		auto & world = m_engine->getModule_World();
		world.addComponentType("Player3D_Component", [](const ParamList & parameters) {
			auto * component = new Player3D_Component();
			return std::make_pair(component->ID, component);
		});
		world.loadWorld("A.map");
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
			auto & world = m_engine->getModule_World();
			if (world.checkIfLoaded())
				m_engine->getModule_Physics().frameTick(deltaTime);
			world.updateSystem(m_freeLookSystem, deltaTime);
			m_engine->getModule_Graphics().frameTick(deltaTime);
			m_engine->getModule_PostProcess().frameTick(deltaTime);
		}
	}


protected:
	// Protected Attributes
	BaseECSSystem * m_freeLookSystem;
};

#endif // GAMESTATE_H