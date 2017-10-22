/*
	Viewport

	- Library specific implementation for creating and managing a rendering context
	- Currently does pretty much nothing except for creating a glfw window and thus a rendering context
*/

#pragma once
#ifndef VIEWPORT
#define VIEWPORT
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "GL\glew.h"

class GLFWwindow;
class Viewport
{
public:
	DELTA_CORE_API ~Viewport();
	DELTA_CORE_API Viewport();
	// Creates the window context and starts up all relevant systems tha depend on an opengl context
	DELTA_CORE_API void Initialize();
	// Returns whether or not the window should close
	DELTA_CORE_API bool ShouldClose() const;

	GLFWwindow *window;
};


#endif // VIEWPORT