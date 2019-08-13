#pragma once
#ifndef EDITOR_MODULE_H
#define EDITOR_MODULE_H

#include "Modules/Engine_Module.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/UI/UI_M.h"


// Forward Declarations
class Editor_Interface;
class Selection_Gizmo;

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
	void clearSelection();
	/***/
	void selectAll();
	/***/
	void setSelection(const std::vector<ecsEntity*>& entities);
	/***/
	const std::vector<ecsEntity*>& getSelection() const;
	/***/
	void mergeSelection();
	/***/
	void groupSelection();
	/***/
	void ungroupSelection();
	/***/
	void makePrefab();
	/***/
	void cutSelection();
	/***/
	void copySelection();
	/***/
	void paste();
	/***/
	void deleteSelection();
	/***/
	void deleteComponent(ecsEntity* handle, const int& componentID);
	/***/
	void addComponent(ecsEntity* handle, const char * name);
	/***/
	void bindFBO();
	/***/
	void bindTexture(const GLuint & offset = 0);
	/***/
	void setGizmoPosition(const glm::vec3 & position);
	/***/
	glm::vec3 getGizmoPosition() const;
	/***/
	void toggleAddToSelection(ecsEntity* entity);
	/***/
	bool hasCopy() const;


private:
	// Private Attributes
	std::string m_currentLevelName = "";
	std::shared_ptr<Editor_Interface> m_editorInterface;
	std::shared_ptr<Selection_Gizmo> m_selectionGizmo;
	GLuint m_fboID = 0, m_texID = 0, m_depthID = 0;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::vector<char> m_copiedData;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // EDITOR_MODULE_H