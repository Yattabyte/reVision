#pragma once
#ifndef GAME_MODULE_H
#define GAME_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsWorld.h"
#include "Modules/Game/Overlays/Overlay.h"
#include "Modules/UI/Basic Elements/UI_Element.h"
#include <memory>


/** A module responsible for Game Logic. */
class Game_Module final : public Engine_Module {
public:
	// Public Enumerations
	enum class Game_State {
		in_pauseMenu,
		in_game,
	};


	// Public (De)Constructors
	/** Destroy this game module. */
	inline ~Game_Module() = default;
	/** Construct a game module.
	@param	engine		the currently active engine. */
	inline explicit Game_Module(Engine* engine) : Engine_Module(engine) {}


	// Public Interface Implementation
	virtual void initialize() noexcept override final;
	virtual void deinitialize() noexcept override final;


	// Public Methods
	/** Tick this module by a specific amount of delta time.
	@param	deltaTime		the amount of time since last frame. */
	void frameTick(const float& deltaTime) noexcept;
	/** Retrieve a reference to the currently active ecsWorld in the editor.
	@return					reference to the currently active ecsWorld. */
	ecsWorld& getWorld() noexcept;
	/** Render any and all of the game module's overlays to the screen.
	@param	deltaTime		the amount of time passed since last frame. */
	void renderOverlays(const float& deltaTime) noexcept;
	/** Show the game. */
	void showGame() noexcept;
	/** Either show or hide the pause menu.
	@param	show			whether to show or hide the pause menu. */
	void showPauseMenu(const bool& show) noexcept;


private:
	// Private Attributes
	Game_State m_gameState = Game_State::in_game;
	ecsSystemList m_Systems;
	ecsWorld m_world;
	std::shared_ptr<UI_Element> m_pauseMenu;
	std::shared_ptr<Overlay> m_loadingRing, m_frameTime;
};

#endif // GAME_MODULE_H
