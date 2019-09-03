#pragma once
#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include "Modules/UI/UI_M.h"


// Forward declarations
class Engine;
class LevelEditor_Module;

/** Allows the user to control the camera whenever not interacting with other UI elements. */
class CameraController : public ImGUI_Element {
public:
	// Public (de)Constructors
	/** Destroy the camera controller. */
	inline ~CameraController() = default;
	/** Construct a camera controller.
	@param	engine		the currently active engine.
	@param	editor		the level editor. */
	CameraController(Engine * engine, LevelEditor_Module * editor);


	// Public Interface Implementation
	virtual void tick(const float & deltaTime) override;


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	LevelEditor_Module * m_editor = nullptr;
	bool m_beginPress = false;
	glm::vec2 m_startPos = glm::vec2(0.0f), m_rotation = glm::vec2(0), m_lastRotation = glm::vec2(0);

};

#endif // CAMERACONTROLLER_H