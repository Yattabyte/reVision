#pragma once
#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include "Modules/Editor/UI/Editor_Interface.h"


// Forward declarations
class Engine;

/** Allows the user to control the camera whenever not interacting with other UI elements. */
class CameraController final : public ImGUI_Element {
public:
	// Public (De)Constructors
	/** Destroy the camera controller. */
	inline ~CameraController() = default;
	/** Construct a camera controller.
	@param	engine		reference to the engine to use. */
	explicit CameraController(Engine& engine) noexcept;


	// Public Interface Implementation
	virtual void tick(const float& deltaTime) noexcept override final;


private:
	// Private Attributes
	Engine& m_engine;
	bool m_beginPress = false;
	glm::vec2 m_startPos = glm::vec2(0.0f), m_rotation = glm::vec2(0), m_lastRotation = glm::vec2(0);
};

#endif // CAMERACONTROLLER_H
