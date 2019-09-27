#pragma once
#ifndef TITLEBAR_H
#define TITLEBAR_H

#include "Modules/UI/UI_M.h"
#include "Assets/Texture.h"


// Forward declarations
class Engine;
class LevelEditor_Module;

/** A level editor UI element acting as a titlebar with File, Edit, menus. */
class TitleBar final : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy this title bar. */
	inline ~TitleBar() = default;
	/** Construct a title bar.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	TitleBar(Engine* engine, LevelEditor_Module* editor);


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) override final;


private:
	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	Shared_Texture m_iconNew, m_iconOpen, m_iconRecent, m_iconSave, m_iconSaveAs, m_iconExit, m_iconUndo, m_iconRedo, m_iconCut, m_iconCopy, m_iconPaste, m_iconDelete, m_iconSettings;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // TITLEBAR_H