/*
	Preference_State

	- A container class that holds the preference state for the engine, such as window size and graphics options
*/



#pragma once
#ifndef PREFERENCE_STATE
#define PREFERENCE_STATE
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Assets\Asset_Config.h"
#include <algorithm>
#include <map>
#include <vector>

using namespace std;

/****************************
----Preferences as ENUMS ----
****************************/
static const enum PREFERENCE_ENUMS {
	C_WINDOW_WIDTH,
	C_WINDOW_HEIGHT,
	C_DRAW_DISTANCE,
	C_TEXTURE_ANISOTROPY,
	C_SHADOW_SIZE_REGULAR,
	C_SHADOW_SIZE_LARGE,
	C_SHADOW_QUALITY
};

/******************************
----Preferences as STRINGS ----
******************************/
static const vector<string> PREFERENCE_STRINGS = {
	"C_WINDOW_WIDTH",
	"C_WINDOW_HEIGHT",
	"C_DRAW_DISTANCE",
	"C_TEXTURE_ANISOTROPY",
	"C_SHADOW_SIZE_REGULAR",
	"C_SHADOW_SIZE_LARGE",
	"C_SHADOW_QUALITY"
};

class Preference_State;
// A class which anything can reimplement when they need to receive information about a preference changing
class DT_ENGINE_API Callback_Container {
public:
	Callback_Container() {};
	virtual ~Callback_Container() {};
	Preference_State *m_preferenceState;

private:
	virtual void Callback(const float &value) = 0;
	friend class Preference_State;
};

class DT_ENGINE_API Preference_State
{
public:
	~Preference_State() {}
	Preference_State(const string &filename = "preferences") {
		LoadFile(filename);
	}
	// Loads a preference file from disk
	void LoadFile(const string &filename) {
		Asset_Loader::load_asset(m_preferences, filename, PREFERENCE_STRINGS, false);
	}
	// Saves the preference file to disk
	void Save() {
		m_preferences->saveConfig();
	}
	// Returns a value for a preference with the given ID
	float GetPreference(const unsigned int &targetKey) const {
		if (m_preferences) 
			return m_preferences->getValue(targetKey);		
		return UNDEFINED_CVAL;
	}
	// Sets a value for a preference with the given ID
	void SetPreference(const unsigned int &targetKey, const float &targetValue) {
		if (m_preferences) {
			m_preferences->setValue(targetKey, targetValue);
			if (m_callbacks.find(targetKey) != m_callbacks.end()) 
				for each (auto &callback in m_callbacks[targetKey])
					callback->Callback(targetValue);			
		}
	}
	// Attaches a callback class which gets triggered when a preference with the given @targetkey is changed
	void AddCallback(const unsigned int &targetKey, Callback_Container *callback) {
		m_callbacks.insert(pair<unsigned int, vector<Callback_Container*>>(targetKey, vector<Callback_Container*>()));
		m_callbacks[targetKey].push_back(callback);
		callback->m_preferenceState = this;
	}
	// Removes the supplied callback from getting triggered when the value at @targetKey changes
	void RemoveCallback(const unsigned int &targetKey, Callback_Container *callback) {
		if (m_callbacks.find(targetKey) != m_callbacks.end()) {
			auto &callback_list = m_callbacks[targetKey];
			callback_list.erase(std::remove_if(begin(callback_list), end(callback_list), [callback](const auto *stored_callback) {
				return (stored_callback == callback);
			}), end(callback_list));
		}
	}
	
private:
	Shared_Asset_Config m_preferences;
	map<unsigned int, vector<Callback_Container*>> m_callbacks;
};

#endif // PREFERENCE_STATE