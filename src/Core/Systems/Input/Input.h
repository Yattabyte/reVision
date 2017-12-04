/*
	Input

	- Manages receiving actual peripheral input devices' input, such as mouse, keyboard, and controllers
	- Performs no interpretation on it
	- Takes in the window to which to track, the engine state to modify, and the binds for which to search
*/



#pragma once
#ifndef SYSTEM_INPUT
#define SYSTEM_INPUT
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\System_Interface.h"
#include "Systems\Input\Input_Binding.h"

class Engine_Package;
class GLFWwindow;

class DT_ENGINE_API System_Input : public System
{
public: 
	~System_Input();
	System_Input(const System_Input_Binding &bind_interface = System_Input_Binding());
	void Initialize(Engine_Package * enginePackage);

	// Check the status of peripheral input devices
	void Update(const float &deltaTime);

	// Callback Functions //
	// To be used in UI interactions ONLY //

	void Callback_CursorPos(GLFWwindow * window, double x, double y);
	void Callback_KeyPress(GLFWwindow * window, int key, int scancode, int action, int mods);
	void Callback_CharMods(GLFWwindow * window, unsigned int codepoint, int mods);
	void Callback_MouseButton(GLFWwindow * window, int button, int action, int mods);
	void Callback_Scroll(GLFWwindow * window, double xoffset, double yoffset);

private:
	System_Input_Binding m_binds; 
};

#endif // SYSTEM_INPUT