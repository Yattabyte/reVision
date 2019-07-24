#pragma once
#ifndef EDITOR_MODULE_H
#define EDITOR_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/Game/Overlays/Overlay.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/UI/UI_M.h"
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
	/***/
	void showEditor();
	/***/
	void exit();
	/***/
	void newLevel();
	/***/
	void openLevel(const std::string & name);
	/***/
	void openLevelDialog();
	/***/
	void saveLevel(const std::string & name);
	/***/
	void saveLevel();
	/***/
	void saveLevelDialog();
	/***/
	void undo();
	/***/
	void redo();
	/***/
	void cut();
	/***/
	void copy();
	/***/
	void paste();
	/***/
	void deleteObject();


private:
	// Private Attributes
	std::string m_currentLevelName = "";
	std::shared_ptr<ImGUI_Element> m_editorInterface;
};

#endif // EDITOR_MODULE_H