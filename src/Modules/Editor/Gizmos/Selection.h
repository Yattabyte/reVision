#pragma once
#ifndef SELECTION_GIZMO_H
#define SELECTION_GIZMO_H

#include "Assets/Auto_Model.h"
#include "Assets/Texture.h"
#include "Assets/Shader.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/Transform.h"
#include <vector>


// Forward Declarations
class Engine;
class LevelEditor_Module;
class Translation_Gizmo;
class Scaling_Gizmo;

/***/
class Selection_Gizmo {
public:
	// Public (de)Constructors
	/***/
	~Selection_Gizmo();
	/***/
	Selection_Gizmo(Engine* engine, LevelEditor_Module* editor);


	// Public Methods
	/***/
	void frameTick(const float& deltaTime);
	/***/
	bool checkInput(const float& deltaTime);
	/***/
	void render(const float& deltaTime);
	/***/
	void setTransform(const Transform& transform);
	/***/
	Transform getTransform() const;
	/***/
	void setSelection(const std::vector<ecsEntity*>& entities);
	/***/
	std::vector<ecsEntity*>& getSelection();



private:
	// Private Methods
	/***/
	bool rayCastMouse(const float& deltaTime);


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	bool m_clicked = false;
	Transform m_transform = glm::vec3(0.0f);
	std::vector<ecsEntity*> m_selection;
	Shared_Texture m_colorPalette;
	Shared_Auto_Model m_model;
	Shared_Shader m_gizmoShader, m_wireframeShader;
	StaticBuffer m_indicatorIndirectBuffer;
	BaseECSSystem* m_pickerSystem;
	unsigned int m_inputMode = 0;
	std::shared_ptr<Translation_Gizmo> m_translationGizmo;
	std::shared_ptr<Scaling_Gizmo> m_scalingGizmo;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SELECTION_GIZMO_H