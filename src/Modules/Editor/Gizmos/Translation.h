#pragma once
#ifndef TRANSLATION_GIZMO_H
#define TRANSLATION_GIZMO_H

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

/** A 3D tool allowing the user to translate a set of selected entities. */
class Translation_Gizmo {
public:
	// Public (de)Constructors
	/** Destroy this gizmo. */
	~Translation_Gizmo();
	/** Construct this gizmo.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	Translation_Gizmo(Engine* engine, LevelEditor_Module* editor);


	// Public Methods
	/** Check for mouse input.
	@param	deltaTime	the amount of time since the last frame. */
	bool checkMouseInput(const float& deltaTime);
	/** Render this gizmo.
	@param	deltaTime	the amount of time since the last frame. */
	void render(const float& deltaTime);
	/** Apply a specific transform.
	@param	transform	the new transform to use. */
	void setTransform(const Transform & transform);


private:
	// Private Methods
	/** Check if the mouse is hovering over any particular element of this gizmo, highlighting it. */
	void checkMouseHover();
	/** Check if the mouse is pressing any particular element of this gizmo, selecting or dragging it. */
	bool checkMousePress();


	// Private Attributes
	Engine* m_engine = nullptr;
	LevelEditor_Module* m_editor = nullptr;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	Transform m_transform;
	enum SelectedAxes : unsigned int {
		NONE	= 0b0000'0000,
		X_AXIS	= 0b0000'0001,
		Y_AXIS	= 0b0000'0010,
		Z_AXIS	= 0b0000'0100,
	};
	float m_renderScale = 0.02f;
	unsigned int m_selectedAxes = NONE, m_hoveredAxes = NONE;
	glm::vec3 m_startingOffset = glm::vec3(0.0f), m_axisDelta = glm::vec3(0.0f), m_hoveredEnds[3], m_direction = glm::vec3(1.0f);
	Shared_Auto_Model m_model;
	Shared_Shader m_gizmoShader, m_axisShader;
	IndirectDraw m_indirectIndicator;
	GLuint m_axisVAO = 0, m_axisVBO = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // TRANSLATION_GIZMO_H