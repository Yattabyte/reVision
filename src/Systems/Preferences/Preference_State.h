#pragma once
#ifndef PREFERENCE_STATE
#define PREFERENCE_STATE
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Assets\Asset_Config.h"
#include "Systems\Preferences\Callback_Interface.h"
#include <algorithm>
#include <map>
#include <vector>

using namespace std;


/**
 * A container class that holds the preference state for the engine, such as window size and graphics options.
 **/
class DT_ENGINE_API Preference_State
{
public:
	// (de)Constructors
	/** Destroy the preference state. */
	~Preference_State() {}
	/** Construct the preference state.
	* @param	filename	an optional relative path to the preference file to load. Defaults to "preferences.cfg" */
	Preference_State(const string & filename = "preferences") {
		LoadFile(filename);
	}


	// Public Methods
	/** Loads a preference file from disk.
	 * @param	filename	the relative path to the preference file to load */
	void LoadFile(const string & filename) {
		Asset_Loader::load_asset(m_preferences, filename, Preference_State::Preference_Strings(), false);
	}
	/** Saves the preference file to disk, using the same filename as when loaded. */
	void Save() {
		m_preferences->saveConfig();
	}
	/** Retrieves a value tied to the supplied preference ID.
	 * @param	targetKey	the preference key to look up
	 * @return	the value tied to the preference supplied */
	float getPreference(const unsigned int & targetKey) const {
		if (m_preferences) 
			return m_preferences->getValue(targetKey);		
		return UNDEFINED_CVAL;
	}
	/** Sets a value for a preference with the given ID.
	 * @param	targetKey	the preference key to set the value to
	 * @param	targetValue	the value to tie to the key supplied */
	void setPreference(const unsigned int & targetKey, const float & targetValue) {
		if (m_preferences) {
			m_preferences->setValue(targetKey, targetValue);
			if (m_callbacks.find(targetKey) != m_callbacks.end()) 
				for each (auto &callback in m_callbacks[targetKey])
					callback->Callback(targetValue);			
		}
	}
	/** Attaches a callback to trigger when a particular preference changes.
	 * @param	targetKey	the preference key to listen for changes
	 * @param	callback	the callback that will fire when the supplied preference changes */
	void addCallback(const unsigned int & targetKey, Callback_Container * callback) {
		m_callbacks.insert(pair<unsigned int, vector<Callback_Container*>>(targetKey, vector<Callback_Container*>()));
		m_callbacks[targetKey].push_back(callback);
		callback->m_preferenceState = this;
	}
	/** Removes a callback from triggering when a particular preference changes.
	* @param	targetKey	the preference key that was listening for changes
	* @param	callback	the callback that should no longer fire when the supplied preference changes */
	void removeCallback(const unsigned int & targetKey, Callback_Container * callback) {
		if (m_callbacks.find(targetKey) != m_callbacks.end()) {
			auto &callback_list = m_callbacks[targetKey];
			callback_list.erase(std::remove_if(begin(callback_list), end(callback_list), [callback](const auto *stored_callback) {
				return (stored_callback == callback);
			}), end(callback_list));
		}
	}


	// Public Static Enumerations
	/** Enumeration for indexing into preferences. */
	static const enum PREFERENCE_ENUM {
		C_WINDOW_WIDTH,
		C_WINDOW_HEIGHT,
		C_BLOOM_STRENGTH,
		C_DRAW_DISTANCE,

		C_SSAO,
		C_SSAO_BLUR_STRENGTH,
		C_SSAO_RADIUS,
		C_SSAO_SAMPLES,

		C_GAMMA,
		C_TEXTURE_ANISOTROPY,
		C_SHADOW_SIZE_REGULAR,
		C_SHADOW_SIZE_LARGE,
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
			"C_SSAO_SAMPLES",

			"C_GAMMA",
			"C_TEXTURE_ANISOTROPY",
			"C_SHADOW_SIZE_REGULAR",
			"C_SHADOW_SIZE_LARGE",
			"C_SHADOW_QUALITY"
		};
		return preferenceStrings;
	};
	
private:
	Shared_Asset_Config m_preferences;
	map<unsigned int, vector<Callback_Container*>> m_callbacks;
};

#endif // PREFERENCE_STATE