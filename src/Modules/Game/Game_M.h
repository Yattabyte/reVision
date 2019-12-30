#pragma once
#ifndef GAME_MODULE_H
#define GAME_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsWorld.h"
#include "Modules/UI/Basic Elements/UI_Element.h"


/** A module responsible for Game Logic. */
class Game_Module final : public Engine_Module {
public:
	// Public Enumerations
	enum class Game_State {
		in_pauseMenu,
		in_game,
	};


	// Public (De)Constructors
	/** Construct a game module.
	@param	engine		reference to the engine to use. */
	explicit Game_Module(Engine& engine);


	// Public Interface Implementation
	void initialize() final;
	void deinitialize() final;


	// Public Methods
	/** Tick this module by a specific amount of delta time.
	@param	deltaTime		the amount of time since last frame. */
	void frameTick(const float& deltaTime);
	/** Retrieve a reference to the currently active ecsWorld in the editor.
	@return					reference to the currently active ecsWorld. */
	ecsWorld& getWorld() noexcept;
	/** Show the game.*/
	void showGame();
	/** Specify a level to load.
	@param	levelName		the level path to open. */
	void loadLevel(const std::string& levelName);
	/** Either show or hide the pause menu.
	@param	show			whether to show or hide the pause menu. */
	void showPauseMenu(const bool& show);


private:
	// Private Attributes
	Game_State m_gameState = Game_State::in_game;
	ecsSystemList m_Systems;
	ecsWorld m_world;
	std::shared_ptr<UI_Element> m_pauseMenu;
};

#endif // GAME_MODULE_H