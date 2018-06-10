#pragma once
#ifndef ENGINE_PACKAGE_H
#define ENGINE_PACKAGE_H

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
class EnginePackage
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
	 * @return		the system requested */
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
	float getPreference(const PreferenceState::Preference & targetKey) const {
		return m_PreferenceState.getPreference(targetKey);
	}
	/** Sets the supplied preference-value to the supplied preference-ID.
	 * @param	targetKey	the ID associated with the supplied preference-value
	 * @param	targetValue	the value to be set to the supplied preference-ID */
	void setPreference(const PreferenceState::Preference & targetKey, const float & targetValue) {
		m_PreferenceState.setPreference(targetKey, targetValue);
	}
	/** Attaches a callback method to be triggered when the supplied preference updates.
	 * @param	targetKey	the preference-ID to which this callback will be attached
	 * @param	pointerID	the pointer to the object owning the function. Used for sorting and removing the callback.
	 * @param	observer	the method to be triggered
	 * @param	<Observer>	the (auto-deduced) signature of the method 
	 * @return				optionally returns the preference value held for this target */
	template <typename Observer>
	float const addPrefCallback(const PreferenceState::Preference & targetKey, void * pointerID, Observer && observer) {
		return m_PreferenceState.addPrefCallback(targetKey, pointerID, observer);
	}
	/** Removes a callback method from triggering when a particular preference changes.
	 * @param	targetKey	the preference key that was listening for changes
	 * @param	pointerID	the pointer to the object owning the callback to be removed */
	void removePrefCallback(const PreferenceState::Preference & targetKey, void * pointerID) {
		m_PreferenceState.removePrefCallback(targetKey, pointerID);
	}


	// Public Attributes
	shared_mutex				m_EngineMutex;
	GLFWwindow				*	m_Context_Rendering;
	Camera						m_Camera;
	ActionState					m_ActionState;
	PreferenceState				m_PreferenceState;
	MappedChar<System*>			m_Systems;
};

#endif // ENGINE_PACKAGE_H