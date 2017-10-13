/*
	Viewport

	- Library specific implementation for creating and managing a rendering context
	- Currently does pretty much nothing except for creating a glfw window and thus a rendering context
*/

#pragma once
#ifndef VIEWPORT
#define VIEWPORT
#ifdef	VIEWPORT_EXPORT
#define VIEWPORT_API __declspec(dllexport)
#else
#define	VIEWPORT_API __declspec(dllimport)
#endif

#include "GL\glew.h"
#include "GLFW\glfw3.h"

class Viewport
{
public:
	VIEWPORT_API ~Viewport();
	VIEWPORT_API Viewport();
	// Creates the window context and starts up all relevant systems tha depend on an opengl context
	VIEWPORT_API void Initialize();
	// Returns whether or not the window should close
	VIEWPORT_API bool ShouldClose() const;

	GLFWwindow *window;
};


#endif // VIEWPORT