#pragma once
#ifndef SELECTION_GIZMO_H
#define SELECTION_GIZMO_H

#include "Assets/Auto_Model.h"
#include "Assets/Texture.h"
#include "Assets/Shader.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Utilities/GL/StaticBuffer.h"
#include <vector>


// Forward Declarations
class Engine;
class LevelEditor_Module;

/***/
class Selection_Gizmo {
public:
	// Public (de)Constructors
	/***/
	~Selection_Gizmo();
	/***/
	Selection_Gizmo(Engine * engine, LevelEditor_Module * editor);


	// Public Methods
	/***/
	void frameTick(const float & deltaTime);
	/***/
	void checkMouseInput(const float & deltaTime);
	/***/
	void render(const float & deltaTime);
	/***/
	void setPosition(const glm::vec3 & position);
	/***/
	glm::vec3 getPosition() const;
	/***/
	void setSelection(const std::vector<ecsEntity*> & entities);
	/***/
	std::vector<ecsEntity*> & getSelection();



private:
	// Private Methods
	/***/
	void rayCastMouse(const float & deltaTime);


	// Private Attributes
	Engine * m_engine = nullptr;
	LevelEditor_Module * m_editor = nullptr;
	bool m_clicked = false;
	glm::vec3 m_position = glm::vec3(0.0f);
	std::vector<ecsEntity*> m_selection;
	Shared_Texture m_colorPalette;
	Shared_Auto_Model m_selIndicator;
	Shared_Shader m_gizmoShader, m_wireframeShader;
	StaticBuffer m_indicatorIndirectBuffer;
	BaseECSSystem * m_pickerSystem;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SELECTION_GIZMO_H