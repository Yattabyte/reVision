#pragma once
#ifndef MAINMENUSTATE_H
#define MAINMENUSTATE_H

#include "States/EngineState.h"
#include "States/Puzzle/PuzzleState.h"
#include "States/Game/GameState.h"
#include "Modules/UI/Macro Elements/StartMenu.h"
#include "Engine.h"


/** Represents the state for the engine while on the main menu. */
class MainMenuState : public EngineState {
public:
	// Public State Enumeration
	enum MenuState {
		on_menu,
		on_game,
		on_puzzle,
		on_exit
	};


	// Public (de)Constructors
	inline ~MainMenuState() {
		m_engine->getModule_World().removeComponentType("MenuCamera_Component");
		m_engine->getModule_UI().clearRootElement();
	}
	inline MainMenuState(Engine * engine) : EngineState(engine) {
		// Register Component Types
		m_engine->getModule_World().addComponentType("MenuCamera_Component", [](const ParamList & parameters) {
			auto * component = new Player3D_Component();
			return std::make_pair(component->ID, component);
		});

		// Create main menu
		m_startMenu = std::make_shared<StartMenu>(m_engine);
		m_startMenu->addCallback(StartMenu::on_start_game, [&]() {
			m_menuState = on_game;
		});
		m_startMenu->addCallback(StartMenu::on_start_puzzle, [&]() {
			m_menuState = on_puzzle;
		});
		m_startMenu->addCallback(StartMenu::on_options, [&]() {
			m_menuState = on_menu;
		});
		m_startMenu->addCallback(StartMenu::on_quit, [&]() {
			m_menuState = on_exit;
		});
		m_engine->getModule_UI().setRootElement(m_startMenu);
		m_engine->setMouseInputMode(Engine::MouseInputMode::NORMAL);
	}


	// Public Interface Implementation
	inline virtual EngineState * handleInput(ActionState & actionState) override {
		switch (m_menuState) {
		case on_game:
			return new GameState(m_engine);
		case on_puzzle:
			return new PuzzleState(m_engine);
		}
		return nullptr;
	}
	inline virtual void handleTick(const float & deltaTime) override {
		m_engine->getModule_Graphics().frameTick(deltaTime);
		m_engine->getModule_PostProcess().frameTick(deltaTime);
		m_engine->getModule_UI().frameTick(deltaTime);
	}


protected:
	// Protected Attributes
	std::shared_ptr<UI_Element> m_startMenu;
	MenuState m_menuState = on_menu;

};

#endif // MAINMENUSTATE_H