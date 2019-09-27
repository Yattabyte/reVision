#pragma once
#ifndef ROTATION_GIZMO_H
#define ROTATION_GIZMO_H

#include "Assets/Auto_Model.h"
#include "Assets/Shader.h"
#include "Modules/ECS/ecsComponent.h"
#include "Modules/ECS/ecsSystem.h"
#include "Utilities/GL/IndirectDraw.h"
#include "Utilities/Transform.h"
#include <vector>


// Forward Declarations
class Engine;
class LevelEditor_Module;

/** A 3D tool allowing the user to rotate a set of selected entities. */
class Rotation_Gizmo {
public:
	// Public (de)Constructors
	/** Destroy this gizmo. */
	~Rotation_Gizmo();
	/** Construct this gizmo.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	Rotation_Gizmo(Engine* engine, LevelEditor_Module* editor);


	// Public Methods
	/** Check for mouse input.
	@param	deltaTime	the amount of time since the last frame. */
	bool checkMouseInput(const float& deltaTime);
	/** Render this gizmo.
	@param	deltaTime	the amount of time since the last frame. */
	void render(const float& deltaTime);
	/** Apply a specific transform.
	@param	transform	the new transform to use. */
	void setTransform(const Transform& transform);


private:
	// Private Methods
	/** Check if the mouse is hovering over any particular element of this gizmo, highlighting it. */
	void checkMouseHover();
	/** Check if the mouse is pressing any particular element of this gizmo, selecting or dragging it. */
	bool checkMousePress();
	/** Update the geometry of the rotation-angle-disk. */
	void updateDisk();


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
	float m_renderScale = 0.02f, m_angleSnapping = 12.5f;
	unsigned int m_selectedAxes = NONE, m_hoveredAxes = NONE;
	glm::vec3 m_hoveredPoint = glm::vec3(0.0f), m_hoveredEnds[3], m_startPoint = glm::vec3(0.0f), m_direction = glm::vec3(1.0f);
	glm::quat m_prevRot = glm::quat(1, 0, 0, 0);
	float m_startingAngle = 0.0f, m_deltaAngle = 0.0f;
	Shared_Auto_Model m_model;
	Shared_Shader m_gizmoShader, m_axisShader;
	IndirectDraw m_indirectIndicator, m_indirectDisk;
	GLuint m_axisVAO = 0, m_axisVBO = 0, m_diskVAO = 0, m_diskVBO = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // ROTATION_GIZMO_H