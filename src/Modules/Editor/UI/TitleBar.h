#pragma once
#ifndef TITLEBAR_H
#define TITLEBAR_H

#include "Modules/Editor/UI/Editor_Interface.h"
#include "Assets/Texture.h"


/** A level editor UI element acting as a title-bar with File, Edit, menus. */
class TitleBar final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Construct a title bar.
	@param	engine		reference to the engine to use.
	@param	editor		reference to the level-editor to use. */
	TitleBar(Engine& engine, LevelEditor_Module& editor);


	// Public Interface Implementation
	void tick(const float& deltaTime) final;


private:
	// Private Attributes
	Engine& m_engine;
	LevelEditor_Module& m_editor;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	Shared_Texture m_iconNew, m_iconOpen, m_iconRecent, m_iconSave, m_iconSaveAs, m_iconExit, m_iconUndo, m_iconRedo, m_iconCut, m_iconCopy, m_iconPaste, m_iconDelete, m_iconSettings;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // TITLEBAR_H