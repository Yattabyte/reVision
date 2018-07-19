#pragma once
#ifndef ENGINE_H
#define ENGINE_H
#define DESIRED_OGL_VER_MAJOR	4
#define DESIRED_OGL_VER_MINOR	5
#define GLEW_STATIC
constexpr char ENGINE_VERSION[]	= "1.1.C";

#include "Assets\Asset.h"
#include "Systems\World\Camera.h"
#include "Systems\Input\ActionState.h"
#include "Systems\Preferences\PreferenceState.h"
#include "Managers\AssetManager.h"
#include "Managers\ModelManager.h"
#include "Managers\MaterialManager.h"
#include "Managers\MessageManager.h"
#include "Utilities\MappedChar.h"
#include <map>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>


class GLFWwindow;
class Camera;
class System;

/**
 * The main game engine object. Encapsulates the entire engine state.
 * The engine is responsible for storing all the system pointers for use through its life.
 **/
class Engine
{
public:
	// Constructors
	/** Destroys the engine. */
	~Engine();
	/** Zero-initialize the engine. */
	Engine();


	// Public Methods
	/** Initializes the engine, and makes this context active for the calling thread.
	 * @return	true if successfully initialized */
	bool initialize();
	/** Shuts down the engine and ceases all threaded activities ASAP. */
	void shutdown();
	/** Ticks the engine's overall simulation by a frame from the main thread. */
	void tick();
	/** Ticks the engine's overall simulation by a frame from a secondary thread. */
	void tickThreaded();
	/** Checks if the engine wants to shut down.
	 * @return	true if engine should shut down */
	bool shouldClose();
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
	/** Creates an asset or uses a cached copy if it has already been created.
	 * @param	sharedAsset		the cointainer to place the asset
	 * @param	args			the rest of the arguments to be used for initialization
	 */
	template <typename SharedAsset, typename... Args>
	void createAsset(SharedAsset & sharedAsset, Args&&... ax) {
		m_AssetManager.create(sharedAsset, std::forward<Args>(ax)...);
	}
	/** Forward a message to the message manager.
	 * @param	input				the message to report */
	void reportMessage(const std::string & input);
	/** Forward an error to the message manager.
	 * @param	error_number		the error number
	 * @param	input				the error to report
	 * @param	additional_input	additional input */
	void reportError(const int & error_number, const std::string & input, const std::string & additional_input = "");
	/** Returns this engine's rendering context. */
	GLFWwindow * getRenderingContext() { return m_Context_Rendering; }
	/** Returns this engine's main camera. */
	Camera * getCamera() { return m_Camera; }
	/** Returns this engine's action state. */
	ActionState & getActionState() { return m_ActionState; }
	/** Returns this engine's preference state. */
	PreferenceState & getPreferenceState() { return m_PreferenceState; }
	/** Returns this engine's asset manager. */
	AssetManager & getAssetManager() { return m_AssetManager; }
	/** Returns this engine's model manager. */
	ModelManager & getModelManager() { return m_modelManager; }
	/** Returns this engine's material manager. */
	MaterialManager & getMaterialManager() { return m_materialManager; }
	/** Returns this engine's message manager. */
	MessageManager & getMessageManager() { return m_messageManager; }


	// Static Methods
	/** Retrieves the application's running directory.
	* @return					std::string of the absolute directory that this executable ran from */
	static std::string Get_Current_Dir();
	/** Check if a given file exists.
	* @param	name			the full file path
	* @return					true if the file exists, false otherwise */
	static bool File_Exists(const std::string & name);


private:
	// Private Attributes
	bool m_Initialized;	
	float m_lastTime; 
	float m_frameAccumulator;
	int m_frameCount;
	GLFWwindow * m_Context_Rendering;
	Camera * m_Camera;
	AssetManager m_AssetManager;
	ActionState	m_ActionState;
	PreferenceState	m_PreferenceState;
	MappedChar<System*>	m_Systems;
	ModelManager m_modelManager;
	MaterialManager m_materialManager;
	MessageManager m_messageManager;
};


#endif // ENGINE_H