#pragma once
#ifndef EDITOR_MODULE_H
#define EDITOR_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/Game/Overlays/Overlay.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/UI/Basic Elements/UI_Element.h"
#include <memory>


/** A level editor module. */
class LevelEditor_Module : public Engine_Module {
public:
	// Public (de)Constructors
	/** Destroy this game module. */
	inline ~LevelEditor_Module() = default;
	/** Construct a game module. */
	inline LevelEditor_Module() = default;


	// Public Interface Implementation
	virtual void initialize(Engine * engine) override;
	virtual void deinitialize() override;
	virtual void frameTick(const float & deltaTime) override;


	// Public Methods
	void start();


private:
	// Private Methods


	// Private Attributes
	std::shared_ptr<UI_Element> m_editorInterface;
};

#endif // EDITOR_MODULE_H