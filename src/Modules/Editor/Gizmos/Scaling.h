#pragma once
#ifndef SCALING_GIZMO_H
#define SCALING_GIZMO_H

#include "Assets/Auto_Model.h"
#include "Assets/Shader.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "Modules/World/ECS/ecsSystem.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Utilities/Transform.h"
#include <vector>


// Forward Declarations
class Engine;
class LevelEditor_Module;

/***/
class Scaling_Gizmo {
public:
	// Public (de)Constructors
	/***/
	~Scaling_Gizmo();
	/***/
	Scaling_Gizmo(Engine* engine, LevelEditor_Module* editor);


	// Public Methods
	/***/
	bool checkMouseInput(const float& deltaTime);
	/***/
	void render(const float& deltaTime);
	/***/
	void setTransform(const Transform& position);


private:
	// Private Methods
	/***/
	void checkMouseHover();
	/***/
	bool checkMousePress();


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	Transform m_transform;
	enum SelectedAxes : unsigned int {
		NONE = 0b0000'0000,
		X_AXIS = 0b0000'0001,
		Y_AXIS = 0b0000'0010,
		Z_AXIS = 0b0000'0100,
	};
	unsigned int m_selectedAxes = NONE, m_hoveredAxes = NONE;
	glm::vec3 m_startingPosition = glm::vec3(0.0f), m_prevScale = glm::vec3(0.0f), m_startingOffset = glm::vec3(0.0f), m_axisDelta = glm::vec3(0.0f), m_hoveredEnds[3], m_direction = glm::vec3(1.0f);
	Shared_Auto_Model m_model;
	Shared_Shader m_gizmoShader, m_axisShader;
	IndirectDraw m_indirectIndicator;
	GLuint m_axisVAO = 0, m_axisVBO = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // SCALING_GIZMO_H