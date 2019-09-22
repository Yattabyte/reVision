#pragma once
#ifndef STARTSCREEN_MODULE_H
#define STARTSCREEN_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/ECS/ecsSystem.h"
#include "Modules/Game/Overlays/Overlay.h"
#include "Modules/UI/Basic Elements/UI_Element.h"
#include <memory>


/** A module responsible for the starting screen logic. */
class StartScreen_Module : public Engine_Module {
public:
	// Public (de)Constructors
	/** Destroy this start screen module. */
	inline ~StartScreen_Module() = default;
	/** Construct a start screen module. */
	inline StartScreen_Module() = default;


	// Public Interface Implementation
	virtual void initialize(Engine * engine) override;
	virtual void deinitialize() override;


	// Public Methods
	/** Display the start menu. */
	void showStartMenu();


private:
	// Private Attributes
	std::shared_ptr<UI_Element> m_startMenu;
};

#endif // STARTSCREEN_MODULE_H