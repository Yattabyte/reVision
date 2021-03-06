#pragma once
#ifndef STARTSCREEN_MODULE_H
#define STARTSCREEN_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsWorld.h"
#include "Modules/UI/Basic Elements/UI_Element.h"


/** A module responsible for the starting screen logic. */
class StartScreen_Module final : public Engine_Module {
public:
	// Public (De)Constructors
	/** Construct a start screen module.
	@param	engine		reference to the engine to use. */
	explicit StartScreen_Module(Engine& engine) noexcept;


	// Public Interface Implementation
	void initialize() final;
	void deinitialize() final;


	// Public Methods
	/** Tick this module by a specific amount of delta time.
	@param	deltaTime		the amount of time since last frame. */
	void frameTick(const float& deltaTime);
	/** Display the start menu. */
	void showStartMenu();


private:
	// Private Attributes
	ecsWorld m_world;
	std::shared_ptr<UI_Element> m_startMenu;
};

#endif // STARTSCREEN_MODULE_H