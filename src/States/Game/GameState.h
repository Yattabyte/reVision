#pragma once
#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "States/EngineState.h"
#include "States/Game/ECS/components.h"
#include "States/Game/ECS/PlayerFreeLook_S.h"
#include "Modules/UI/Macro Elements/PauseMenu.h"
#include "Engine.h"


/** Represents an engine state for the "game". */
class GameState : public EngineState {
public:
	// Public State Enumeration
	enum MenuState {
		in_game,
		in_menu,
	};


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

		// Tap-in to preference changes
		auto & preferences = m_engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
			m_renderSize.x = (int)f;
		});
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
			m_renderSize.y = (int)f;
		});

		// Create main menu
		m_pauseMenu = std::make_shared<PauseMenu>(m_engine);
		m_pauseMenu->addCallback(PauseMenu::on_resume_game, [&]() {
			m_engine->getModule_UI().clearRootElement();
		});
		m_pauseMenu->addCallback(PauseMenu::on_options, [&]() {
			m_menuState = in_menu;
		});
		m_pauseMenu->addCallback(PauseMenu::on_quit, [&]() {
			m_engine->getModule_UI().clearRootElement();
		});

		m_engine->setMouseInputMode(Engine::MouseInputMode::FREE_LOOK);
	}


	// Public Interface Implementation
	inline virtual EngineState * handleInput(ActionState & actionState) override {
		if (m_menuState == in_game) {
			// Check if we should enable the overlay
			if (actionState.isAction(ActionState::ESCAPE)) {
				m_engine->setMouseInputMode(Engine::MouseInputMode::NORMAL);
				m_engine->getModule_UI().setRootElement(m_pauseMenu);
				m_menuState = in_menu;
			}
		}
		else if (m_menuState == in_menu) {		
			// Update Mouse Position
			m_mouseEvent.m_xPos = (double)actionState[ActionState::MOUSE_X];
			m_mouseEvent.m_yPos = m_renderSize.y - (double)actionState[ActionState::MOUSE_Y];
			m_mouseEvent.m_button = GLFW_MOUSE_BUTTON_LEFT;
			m_mouseEvent.m_action = (int)actionState[ActionState::MOUSE_L];
			m_engine->getModule_UI().applyMouseEvent(m_mouseEvent);

			// Check if we should disable the overlay
			if (actionState.isAction(ActionState::ESCAPE)) {
				m_engine->setMouseInputMode(Engine::MouseInputMode::FREE_LOOK);
				m_engine->getModule_UI().clearRootElement();
				m_menuState = in_game;
			}				
		}
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
			m_engine->getModule_UI().frameTick(deltaTime);
		}
	}


protected:
	// Protected Attributes
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	BaseECSSystem * m_freeLookSystem;
	std::shared_ptr<UI_Element> m_pauseMenu;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	MouseEvent m_mouseEvent;
	MenuState m_menuState = in_game;
};

#endif // GAMESTATE_H