#pragma once
#ifndef GAME_MODULE_H
#define GAME_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/Game/Overlays/Overlay.h"
#include "Modules/UI/Basic Elements/UI_Element.h"
#include <memory>


/** A module responsible for Game Logic. */
class Game_Module : public Engine_Module {
public:
	// Public Enumerations
	enum Game_State {
		in_pauseMenu,
		in_game,
	};


	// Public (de)Constructors
	/** Destroy this game module. */
	inline ~Game_Module() = default;
	/** Construct a game module. */
	inline Game_Module() = default;


	// Public Interface Implementation
	virtual void initialize(Engine * engine) override;
	virtual void deinitialize() override;
	virtual void frameTick(const float & deltaTime) override;


	// Public Methods
	/** Render any and all of the game module's overlays to the screen.
	@param	deltaTime	the amount of time passed since last frame. */
	void renderOverlays(const float & deltaTime);
	/** Show the game. */
	void showGame();
	/** Either show or hide the pause menu.
	@param	show		whether to show or hide the pause menu. */
	void showPauseMenu(const bool & show);


private:
	// Private Attributes
	Game_State m_gameState = in_game;
	ecsSystemList m_Systems;
	std::shared_ptr<UI_Element> m_pauseMenu;
	std::shared_ptr<Overlay> m_loadingRing, m_frameTime;
};

#endif // GAME_MODULE_H