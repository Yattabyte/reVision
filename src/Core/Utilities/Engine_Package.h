#include <map>
#include <shared_mutex>
#include <thread>
#include <vector>
#include "Utilities\Action_State.h"

class GLFWwindow;
class Camera;
class System;

class Engine_Package 
{
public:
	shared_mutex					m_EngineMutex;
	GLFWwindow					*	m_Context_Rendering;
	Camera						*	m_Camera;
	map<const char*, System*>		m_Systems;
	Action_State					m_Action_State;
	float							window_width,
		window_height;

	Engine_Package() {
		m_Context_Rendering = nullptr;
		m_Camera = nullptr;
	}

	bool FindSubSystem(const char*c) { 
		if (m_Systems.find(c) == m_Systems.end()) 
			return false; 
		else
			return true; 
	}

	template <typename T>
	T * GetSubSystem(const char*c) {
		return (T*)m_Systems[c];
	}
};