#pragma once
#ifndef STARTSCREEN_MODULE_H
#define STARTSCREEN_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsWorld.h"
#include "Modules/Game/Overlays/Overlay.h"
#include "Modules/UI/Basic Elements/UI_Element.h"
#include <memory>


/** A module responsible for the starting screen logic. */
class StartScreen_Module final : public Engine_Module {
public:
	// Public (De)Constructors
	/** Destroy this start screen module. */
	inline ~StartScreen_Module() = default;
	/** Construct a start screen module.
	@param	engine		the currently active engine. */
	inline explicit StartScreen_Module(Engine& engine) : Engine_Module(engine) {}


	// Public Interface Implementation
	virtual void initialize() noexcept override final;
	virtual void deinitialize() noexcept override final;


	// Public Methods
	/** Tick this module by a specific amount of delta time.
	@param	deltaTime		the amount of time since last frame. */
	void frameTick(const float& deltaTime) noexcept;
	/** Display the start menu. */
	void showStartMenu() noexcept;


private:
	// Private Attributes
	ecsWorld m_world;
	std::shared_ptr<UI_Element> m_startMenu;
};

#endif // STARTSCREEN_MODULE_H
