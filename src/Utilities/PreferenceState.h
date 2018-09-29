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
	~PreferenceState() = default;
	/** Construct the preference state.
	@param	engine		the engine
	@param	filename	an optional relative path to the preference file to load. Defaults to "preferences.cfg" */
	PreferenceState(Engine * engine, const std::string & filename = "preferences");
	

	// Public Static Enumerations
	/** Enumeration for indexing into preferences. */
	const enum Preference {
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

		C_RH_BOUNCE_SIZE,

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

			"C_RH_BOUNCE_SIZE",

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
	template <typename T>
	const T getPreference(const Preference & targetKey) const	{
		if (m_preferences)
			return (T)m_preferences->getValue(targetKey);
		float undefinedCVAL = UNDEFINED_CVAL;
		return (T)(undefinedCVAL);
	}
	/** Sets a value for a preference with the given ID.
	@param	targetKey	the preference key to set the value to
	@param	targetValue	the value to tie to the key supplied 
	@param	<T>			the value type to use */
	template <typename T>
	inline void setPreference(const Preference & targetKey, const T & targetValue)	{
		const float castValue = (float)targetValue;
		if (m_preferences) {
			m_preferences->setValue(targetKey, castValue);

			// Call callbacks
			size_t index = 0;
			if (m_callbacks.find(targetKey) != m_callbacks.end())
				for each (const auto &pair in m_callbacks[targetKey]) {
					if (pair.first)
						pair.second(castValue);
					else
						m_callbacks[targetKey].erase(m_callbacks[targetKey].begin() + index);
					index++;
				}
			
		}
	}
	/** Attaches a callback method to be triggered when the supplied preference updates.
	@param	targetKey	the preference-ID to which this callback will be attached
	@param	alive		the
	@param	observer	the method to be triggered
	@return				optionally returns the preference value held for this target */
	template <typename T, typename Observer>
	const T addPrefCallback(const Preference & targetKey, const std::shared_ptr<bool> & alive, Observer&& observer) {
		m_callbacks[targetKey].emplace_back(std::make_pair(alive, std::function<void(float)>(std::forward<Observer>(observer))));
		return getPreference<T>(targetKey);
	}

	
private:
	Engine * m_engine = nullptr;
	Shared_Asset_Config m_preferences;
	std::map< Preference, std::vector<std::pair<std::shared_ptr<bool>, std::function<void(float)>>> > m_callbacks;
};

#endif // PREFERENCE_STATE_H