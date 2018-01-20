#include "Systems\World\Camera.h"
#include "Systems\Input\Action_State.h"
#include "Systems\Preferences\Preference_State.h"
#include <map>
#include <shared_mutex>
#include <thread>
#include <vector>

class GLFWwindow;
class System;

class Engine_Package 
{
public:
	shared_mutex							m_EngineMutex;
	GLFWwindow							*	m_Context_Rendering;
	Camera									m_Camera;
	Action_State							m_Action_State;
	Preference_State						m_Preference_State;
	map<const char*, System*, cmp_str>		m_Systems;	

	Engine_Package() {
		m_Context_Rendering = nullptr;
	}

	// Returns if it can find a subsystem with the given name
	bool FindSubSystem(const char*c) { 
		if (m_Systems.find(c) == m_Systems.end()) 
			return false; 
		else
			return true; 
	}
	// Forcefully returns a typecasted subsystem with the given name
	template <typename T> T * GetSubSystem(const char*c) {
		return (T*)m_Systems[c];
	}
	// Returns a value for a preference with the given ID
	float GetPreference(const unsigned int &targetKey) const {
		return m_Preference_State.GetPreference(targetKey);
	}
	// Sets a value for a preference with the given ID
	void SetPreference(const unsigned int &targetKey, const float &targetValue) {
		m_Preference_State.SetPreference(targetKey, targetValue);
	}
	// Attaches a callback class which gets triggered when a preference with the given @targetkey is changed
	void AddCallback(const unsigned int &targetKey, Callback_Container *callback) {
		m_Preference_State.AddCallback(targetKey, callback);
	}
	// Removes the supplied callback from getting triggered when the value at @targetKey changes
	void RemoveCallback(const unsigned int &targetKey, Callback_Container *callback) {
		m_Preference_State.RemoveCallback(targetKey, callback);
	}
};