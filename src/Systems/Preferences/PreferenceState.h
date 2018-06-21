#pragma once
#ifndef PREFERENCE_STATE_H
#define PREFERENCE_STATE_H

#include "Assets\Asset_Config.h"
#include <algorithm>
#include <functional>
#include <map>
#include <vector>
#include <utility>

using namespace std;


/**
 * A container class that holds the preference state for the engine, such as window size and graphics options.
 **/
class PreferenceState
{
public:
	// (de)Constructors
	/** Destroy the preference state. */
	~PreferenceState() {}
	/** Construct the preference state.
	* @param	filename	an optional relative path to the preference file to load. Defaults to "preferences.cfg" */
	PreferenceState(const string & filename = "preferences") {
		LoadFile(filename);
	}
	

	// Public Static Enumerations
	/** Enumeration for indexing into preferences. */
	static const enum Preference {
		C_WINDOW_WIDTH,
		C_WINDOW_HEIGHT,
		C_BLOOM_STRENGTH,
		C_DRAW_DISTANCE,

		C_SSAO,
		C_SSAO_BLUR_STRENGTH,
		C_SSAO_RADIUS,
		C_SSAO_QUALITY,

		C_GAMMA,
		C_TEXTURE_ANISOTROPY,
		C_SHADOW_SIZE_DIRECTIONAL,
		C_SHADOW_SIZE_POINT,
		C_SHADOW_SIZE_SPOT,
		C_SHADOW_QUALITY
	};


	// Public Static Methods
	/* Retrieve a static list of all user-preferences.
	* @return	vector of preference names as strings */
	static vector<string> Preference_Strings() {
		static const vector<string> preferenceStrings = {
			"C_WINDOW_WIDTH",
			"C_WINDOW_HEIGHT",
			"C_BLOOM_STRENGTH",
			"C_DRAW_DISTANCE",

			"C_SSAO",
			"C_SSAO_BLUR_STRENGTH",
			"C_SSAO_RADIUS",
			"C_SSAO_QUALITY",

			"C_GAMMA",
			"C_TEXTURE_ANISOTROPY",
			"C_SHADOW_SIZE_DIRECTIONAL",
			"C_SHADOW_SIZE_POINT",
			"C_SHADOW_SIZE_SPOT",
			"C_SHADOW_QUALITY"
		};
		return preferenceStrings;
	};


	// Public Methods
	/** Loads a preference file from disk.
	 * @param	filename	the relative path to the preference file to load */
	void LoadFile(const string & filename) {
		Asset_Config::Create(m_preferences, filename, PreferenceState::Preference_Strings(), false);
	}
	/** Saves the preference file to disk, using the same filename as when loaded. */
	void Save() {
		m_preferences->saveConfig();
	}
	/** Retrieves a value tied to the supplied preference ID.
	 * @param	targetKey	the preference key to look up
	 * @return				the value tied to the preference supplied */
	float getPreference(const Preference & targetKey) const {
		if (m_preferences) 
			return m_preferences->getValue(targetKey);		
		return UNDEFINED_CVAL;
	}
	/** Sets a value for a preference with the given ID.
	 * @param	targetKey	the preference key to set the value to
	 * @param	targetValue	the value to tie to the key supplied */
	void setPreference(const Preference & targetKey, const float & targetValue) {
		if (m_preferences) {
			m_preferences->setValue(targetKey, targetValue);
			if (m_callbacks.find(targetKey) != m_callbacks.end())
				for each (const auto &observer in m_callbacks[targetKey])
					observer.second(targetValue);
		}
	}	
	/** Attaches a callback method to be triggered when the supplied preference updates.
	 * @param	targetKey	the preference-ID to which this callback will be attached
	 * @param	pointerID	the pointer to the object owning the function. Used for sorting and removing the callback.
	 * @param	observer	the method to be triggered
	 * @param	<Observer>	the (auto-deduced) signature of the method
	 * @return				optionally returns the preference value held for this target */
	template <typename Observer>
	float const addPrefCallback(const Preference & targetKey, void * pointerID, Observer&& observer) {
		m_callbacks.insert(pair<Preference, map<void*, function<void(float)>>>(targetKey, map<void*, function<void(float)>>()));
		m_callbacks[targetKey].insert(pair<void*, function<void(float)>>(pointerID, function<void(float)>()));
		m_callbacks[targetKey][pointerID] = forward<Observer>(observer);
		return getPreference(targetKey);
	}
	/** Removes a callback method from triggering when a particular preference changes.
	 * @param	targetKey	the preference key that was listening for changes
	 * @param	pointerID	the pointer to the object owning the callback to be removed */
	void removePrefCallback(const Preference & targetKey, void * pointerID) {
		if (m_callbacks.find(targetKey) != m_callbacks.end()) {
			auto &specific_map = m_callbacks[targetKey];
			if (specific_map.find(pointerID) != specific_map.end()) 
				specific_map.erase(specific_map.find(pointerID));			
		}
	}

	
private:
	Shared_Asset_Config m_preferences;
	map<Preference, map<void*, function<void(float)>>> m_callbacks;
};

#endif // PREFERENCE_STATE_H