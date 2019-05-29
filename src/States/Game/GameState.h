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

		// Create main menu
		m_pauseMenu = std::make_shared<PauseMenu>(m_engine);
		m_pauseMenu->addCallback(PauseMenu::on_resume_game, [&]() {
			showPauseMenu(false);
			m_pauseMenu->setVisible(true);
		});
		m_pauseMenu->addCallback(PauseMenu::on_options, [&]() {
			m_paused = true;
		});
		m_pauseMenu->addCallback(PauseMenu::on_quit, [&]() {
			showPauseMenu(false);
			m_engine->getModule_UI().clear();
		});

		m_engine->setMouseInputMode(Engine::MouseInputMode::FREE_LOOK);
	}


	// Public Interface Implementation
	inline virtual void handleTick(const float & deltaTime) override {
		// Handle GLOBAL user input
		auto & actionState = m_engine->getActionState();
		if (m_engine->getMouseInputMode() == Engine::FREE_LOOK) {
			actionState[ActionState::LOOK_X] = actionState[ActionState::MOUSE_X];
			actionState[ActionState::LOOK_Y] = actionState[ActionState::MOUSE_Y];
		}
		// Check if we should show the overlay
		if (actionState.isAction(ActionState::UI_ESCAPE) == ActionState::PRESS)
			showPauseMenu(!m_paused);

		// Enable physics once world is fully loaded
		auto & world = m_engine->getModule_World();
		if (world.checkIfLoaded())
			m_engine->getModule_Physics().frameTick(deltaTime);

		// Appy our free-look system
		world.updateSystem(m_freeLookSystem, deltaTime);

		// Render graphics-related systems
		m_engine->getModule_Graphics().frameTick(deltaTime);
		m_engine->getModule_PostProcess().frameTick(deltaTime);
		m_engine->getModule_UI().frameTick(deltaTime);		
	}


protected:
	// Protected Methods
	void showPauseMenu(const bool & show) {
		if (show) {
			m_engine->setMouseInputMode(Engine::MouseInputMode::NORMAL);
			m_engine->getModule_UI().pushRootElement(m_pauseMenu);
			m_paused = true;
		}
		else {
			m_engine->setMouseInputMode(Engine::MouseInputMode::FREE_LOOK);
			m_engine->getModule_UI().clear();
			m_paused = false;
		}
	}


	// Protected Attributes
	BaseECSSystem * m_freeLookSystem;
	std::shared_ptr<UI_Element> m_pauseMenu;
	bool m_paused = false;
};

#endif // GAMESTATE_H