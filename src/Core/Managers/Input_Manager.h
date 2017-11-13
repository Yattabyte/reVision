/*
	Input_Manager

	- Receives input from the main GLFWwindow and uses it to modify the internal engine state
	- Requires GLFW
*/

#pragma once
#ifndef INPUT_MANAGER
#define INPUT_MANAGER
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "GLFW\glfw3.h"

namespace Input_Manager {
	void CursorPosCallback(GLFWwindow* window, double x, double y);
	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void CharModsCallback(GLFWwindow* window, unsigned int codepoint, int mods);
	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
}

#endif // INPUT_MANAGER