#pragma once
#ifndef PREFERENCE_STATE_H
#define PREFERENCE_STATE_H

#include "Assets\Asset_Config.h"
#include <algorithm>
#include <functional>
#include <map>
#include <vector>
#include <utility>


class Engine;

/** A container class that holds the preference state for the engine, such as window size and graphics options. */
class PreferenceState {
public:
	// (de)Constructors
	/** Destroy the preference state. */
	~PreferenceState();
	/** Construct the preference state.
	@param	engine		the engine
	@param	filename	an optional relative path to the preference file to load. Defaults to "preferences.cfg" */
	PreferenceState(Engine * engine, const std::string & filename = "preferences");
	

	// Public Static Enumerations
	/** Enumeration for indexing into preferences. */
	static const enum Preference {
		C_WINDOW_WIDTH,
		C_WINDOW_HEIGHT,
		C_WINDOW_USE_MONITOR_RATE,
		C_WINDOW_REFRESH_RATE,
		C_VSYNC,

		C_BLOOM,
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
		C_SHADOW_QUALITY,

		C_ENVMAP_SIZE,

		C_SSR,

		C_FXAA,
	};


	// Public Static Methods
	/* Retrieve a static list of all user-preferences.
	@return	std::vector of preference names as strings */
	static std::vector<std::string> Preference_Strings() {
		static const std::vector<std::string> preferenceStrings = {
			"C_WINDOW_WIDTH",
			"C_WINDOW_HEIGHT",
			"C_WINDOW_USE_MONITOR_RATE",
			"C_WINDOW_REFRESH_RATE",
			"C_VSYNC",

			"C_BLOOM",
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
			"C_SHADOW_QUALITY",

			"C_ENVMAP_SIZE",

			"C_SSR",

			"C_FXAA"
		};
		return preferenceStrings;
	};


	// Public Methods
	/** Loads a preference file from disk.
	@param	filename	the relative path to the preference file to load */
	void loadFile(const std::string & filename);
	/** Saves the preference file to disk, using the same filename as when loaded. */
	void save();
	/** Retrieves a value tied to the supplied preference ID.
	@param	targetKey	the preference key to look up
	@return				the value tied to the preference supplied */
	float getPreference(const Preference & targetKey) const;
	/** Sets a value for a preference with the given ID.
	@param	targetKey	the preference key to set the value to
	@param	targetValue	the value to tie to the key supplied */
	void setPreference(const Preference & targetKey, const float & targetValue);
	/** Attaches a callback method to be triggered when the supplied preference updates.
	@param	targetKey	the preference-ID to which this callback will be attached
	@param	pointerID	the pointer to the object owning the function. Used for sorting and removing the callback.
	@param	observer	the method to be triggered
	@param	<Observer>	the (auto-deduced) signature of the method
	@return				optionally returns the preference value held for this target */
	template <typename Observer>
	float const addPrefCallback(const Preference & targetKey, void * pointerID, Observer&& observer) {
		m_callbacks.insert(std::pair<Preference, std::map<void*, std::function<void(float)>>>(targetKey, std::map<void*, std::function<void(float)>>()));
		m_callbacks[targetKey].insert(std::pair<void*, std::function<void(float)>>(pointerID, std::function<void(float)>()));
		m_callbacks[targetKey][pointerID] = std::forward<Observer>(observer);
		return getPreference(targetKey);
	}
	/** Removes a callback method from triggering when a particular preference changes.
	@param	targetKey	the preference key that was listening for changes
	@param	pointerID	the pointer to the object owning the callback to be removed */
	void removePrefCallback(const Preference & targetKey, void * pointerID) {
		if (m_callbacks.find(targetKey) != m_callbacks.end()) {
			auto &specific_map = m_callbacks[targetKey];
			if (specific_map.find(pointerID) != specific_map.end()) 
				specific_map.erase(specific_map.find(pointerID));			
		}
	}

	
private:
	Engine * m_engine;
	Shared_Asset_Config m_preferences;
	std::map<Preference, std::map<void*, std::function<void(float)>>> m_callbacks;
};

#endif // PREFERENCE_STATE_H