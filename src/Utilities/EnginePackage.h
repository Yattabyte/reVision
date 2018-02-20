#pragma once
#ifndef ENGINE_PACKAGE
#define ENGINE_PACKAGE
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\Camera.h"
#include "Systems\Input\ActionState.h"
#include "Systems\Preferences\PreferenceState.h"
#include "Utilities\MappedChar.h"
#include <shared_mutex>
#include <thread>

class GLFWwindow;
class System;


/** 
 * Holds the engine state in a manner that can be shared among the entire program.
 * Useful for sharing systems, configurations, and preferences all throughout.
 **/
class DT_ENGINE_API EnginePackage
{
public:
	// (de)Constructors
	/** Zero-Initialization Constructor */
	EnginePackage() {
		m_Context_Rendering = nullptr;
	}


	// Public Methods
	/** Searches for a subsystem with the given name.
	 * @param	c	a const char array name of the desired system to find
	 * @return		true if it can find the system, false otherwise */
	bool findSubSystem(const char * c) { 
		return m_Systems.find(c);
	}
	/** Returns a type-casted subsystem that matches the given name.
	 * @param	c	a const char array name of the desired system to find
	 * @return		true if it can find the system, false otherwise */
	template <typename T> T * getSubSystem(const char * c) {
		return (T*)m_Systems[c];
	}
	/** Returns the preference-value associated with the supplied preference-ID.
	 * @param	targetKey	the ID associated with the desired preference-value
	 * @return				the value associated with the supplied preference-ID */
	float getPreference(const unsigned int & targetKey) const {
		return m_PreferenceState.getPreference(targetKey);
	}
	/** Sets the supplied preference-value to the supplied preference-ID.
	 * @param	targetKey	the ID associated with the supplied preference-value
	 * @param	targetValue	the value to be set to the supplied preference-ID */
	void setPreference(const unsigned int & targetKey, const float & targetValue) {
		m_PreferenceState.setPreference(targetKey, targetValue);
	}
	/** Attaches a callback class to be triggered when the supplied preference updates.
	 * @param	targetKey	the preference-ID to which this callback will be attached
	 * @param	callback	the callback pointer to be attached */
	void addCallback(const unsigned int & targetKey, Callback_Container * callback) {
		m_PreferenceState.addCallback(targetKey, callback);
	}
	/** Removes the supplied callback from getting triggered when the supplied preference updates.
	 * @param	targetKey	the preference-ID to which this callback was attached
	 * @param	callback	the callback pointer that was attached 
	 * @note	- Safety Measures:
	 *				-# safe to call even if the callback isn't attached 
	 *				-# will check and remove redundant callbacks (if was attached more than once to this preference) */
	void removeCallback(const unsigned int & targetKey, Callback_Container * callback) {
		m_PreferenceState.removeCallback(targetKey, callback);
	}


	// Public Attributes
	shared_mutex				m_EngineMutex;
	GLFWwindow				*	m_Context_Rendering;
	Camera						m_Camera;
	ActionState					m_ActionState;
	PreferenceState				m_PreferenceState;
	MappedChar<System*>			m_Systems;
};

#endif // ENGINE_PACKAGE