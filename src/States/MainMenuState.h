#pragma once
#ifndef MAINMENUSTATE_H
#define MAINMENUSTATE_H

#include "States/EngineState.h"
#include "States/Puzzle/PuzzleState.h"
#include "States/Game/GameState.h"
#include "Engine.h"


class MainMenuState : public EngineState {
public:
	// Public (de)Constructors
	inline ~MainMenuState() = default;
	inline MainMenuState(Engine * engine) : EngineState(engine) {
		m_engine->registerECSConstructor("Player3D_Component", new Player3D_Constructor());
		//m_engine->getModule_World().loadWorld("background.map");
		glfwSetInputMode(m_engine->getRenderingContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
			m_renderSize.x = (int)f;
		});
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
			m_renderSize.y = (int)f;
		});
	}


	// Public Interface Implementation
	inline virtual EngineState * handleInput(ActionState & actionState) override {
		// Update Mouse Position
		m_mouseEvent.m_xPos = (double)actionState[ActionState::MOUSE_X];
		m_mouseEvent.m_yPos = m_renderSize.y - (double)actionState[ActionState::MOUSE_Y];

		// Update Mouse Buttons
		m_mouseEvent.m_button = GLFW_MOUSE_BUTTON_LEFT;
		m_mouseEvent.m_action = (int)actionState[ActionState::MOUSE_L];

		m_engine->getModule_UI().applyMouseEvent(m_mouseEvent);

		switch (m_engine->getModule_UI().getMenuState()) {
		case UI_Module::on_game:
			return new GameState(m_engine);
		case UI_Module::on_puzzle:
			return new PuzzleState(m_engine);
		}
		return nullptr;
	}
	inline virtual void handleTick(const float & deltaTime) override {
		m_engine->getModule_Graphics().setActiveCamera(0);
		m_engine->getModule_Graphics().frameTick(deltaTime);
		m_engine->getModule_PostProcess().frameTick(deltaTime);
		m_engine->getModule_UI().frameTick(deltaTime);
	}


protected:
	// Protected Attributes
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	glm::ivec2 m_renderSize = glm::ivec2(1);
	MouseEvent m_mouseEvent;
};

#endif // MAINMENUSTATE_H