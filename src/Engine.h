#pragma once
#ifndef ENGINE_H
#define ENGINE_H

// Managers
#include "Managers/AssetManager.h"
#include "Managers/MessageManager.h"
#include "Managers/SoundManager.h"

// Modules
#include "Modules/ECS/ECS_M.h"
#include "Modules/StartScreen/StartScreen_M.h"
#include "Modules/Game/Game_M.h"
#include "Modules/Editor/Editor_M.h"
#include "Modules/Graphics/Graphics_M.h"
#include "Modules/Physics/Physics_M.h"
#include "Modules/UI/UI_M.h"

// Utilities
#include "Utilities/ActionState.h"
#include "Utilities/InputBinding.h"
#include "Utilities/PreferenceState.h"
#include "Utilities/MappedChar.h"

// Other
#include <string>


constexpr char ENGINE_VERSION[] = "4.17.13";
struct GLFWwindow;

/** The main game engine object. Encapsulates the entire engine state. */
class Engine {
public:
	// Public Enumerations
	/** Different mouse input modes. */
	enum class MouseInputMode {
		NORMAL,
		FREE_LOOK
	};


		// Public (De)Constructors
	/** Destroys the game engine. */
	~Engine() noexcept;
	/** Construct the game engine. */
	Engine() noexcept;


private:
	// Private Initialization Methods
	/** Initialize the window for this application. */
	void initWindow() noexcept;
	/** Initialize the auxiliary processing threads. */
	void initThreads() noexcept;
	/** Print the engine boiler-plate text to the message manager. */
	void printBoilerPlate() noexcept;


public:
	// Public Methods
	/** Ticks the engine's overall simulation by a frame from the main thread. */
	void tick() noexcept;
	/** Ticks the engine's overall simulation by a frame from a secondary thread.
	@param	exitObject	object signaling when to close the thread.
	@param	window		pointer to the window object. */
	void tickThreaded(std::future<void> exitObject, GLFWwindow* const window) noexcept;
	/** Checks if the engine wants to shut down.
	@return	true if engine should shut down. */
	bool shouldClose() noexcept;
	/** Tells the engine to shut down. */
	void shutDown() noexcept;
	/** Set the input mode for the mouse, useful for changing between 2D and 3D views.
	@param	mode		the new mouse input mode to use. */
	void setMouseInputMode(const MouseInputMode& mode) noexcept;
	/** Switch the UI over to the main menu. */
	void goToMainMenu() noexcept;
	/** Switch the UI over to the game. */
	void goToGame() noexcept;
	/** Switch the UI over to the level editor. */
	void goToEditor() noexcept;


	// Public Accessors
	/** Retrieve the mouse input mode.
	@return				the mouse input mode, such as free-look or normal cursor. */
	inline MouseInputMode getMouseInputMode() const noexcept { return m_mouseInputMode; };
	/** Retrieve the current time. 
	@return				the current time. */
	static float getTime() noexcept;
	/** Retrieve a list of available resolutions. 
	@return				vector of supported resolutions. */
	static std::vector<glm::ivec3> getResolutions() noexcept;
	/** Retrieve this engine's action state.
	@return				the engine's action state. */
	inline ActionState& getActionState() noexcept { return m_actionState; }
	/** Retrieve this engine's preference state. 
	@return				this engine's preference state. */
	inline PreferenceState& getPreferenceState() noexcept { return m_preferenceState; }
	/** Retrieve this engine's rendering context. 
	@return				this engine's rendering context. */
	inline GLFWwindow* getContext() const noexcept { return m_window; }


	// Manager Accessors
	/** Retrieve this engine's asset manager. 
	@return				this engine's asset manager. */
	inline AssetManager& getManager_Assets() noexcept { return m_assetManager; }
	/** Retrieve this engine's message manager. 
	@return				this engine's message manager. */
	inline MessageManager& getManager_Messages() noexcept { return m_messageManager; }
	/** Retrieve this engine's sound manager. 
	@return				this engine's sound manager. */
	inline SoundManager& getManager_Sounds() noexcept { return m_soundManager; }


	// Module Accessors
	/** Retrieve this engine's ECS module.
	@return				this engine's ECS module. */
	inline ECS_Module& getModule_ECS() noexcept { return m_moduleECS; }
	/** Retrieve this engine's game module.
	@return				this engine's game module. */
	inline Game_Module& getModule_Game() noexcept { return m_moduleGame; }
	/** Retrieve this engine's editor module.
	@return				this engine's level editor module. */
	inline LevelEditor_Module& getModule_LevelEditor() noexcept { return m_moduleEditor; }
	/** Retrieve this engine's graphics module.
	@return				this engine's graphics module. */
	inline Graphics_Module& getModule_Graphics() noexcept { return m_moduleGraphics; }
	/** Retrieve this engine's user-interface module.
	@return				this engine's UI module. */
	inline UI_Module& getModule_UI() noexcept { return m_moduleUI; }
	/** Retrieve this engine's physics module.
	@return				this engine's physics module. */
	inline Physics_Module& getModule_Physics() noexcept { return m_modulePhysics; }


	// Static Methods
	/** Retrieves the application's running directory.
	@return					std::string of the absolute directory that this executable ran from */
	static std::string Get_Current_Dir() noexcept;
	/** Check if a given file exists, relative to the application directory.
	@param	name			the full file path
	@return					true if the file exists, false otherwise */
	static bool File_Exists(const std::string& name) noexcept;


private:
	// Private Methods
	/** Updates the window attributes. */
	void configureWindow() noexcept;


	// Private Attributes
	enum class Engine_State {
		in_startMenu,
		in_game,
		in_editor
	} m_engineState = Engine_State::in_startMenu;
	float m_lastTime = 0.0f;
	float m_frameAccumulator = 0.0f;
	float m_refreshRate = 120.0f;
	float m_useFullscreen = 1.0f;
	float m_vsync = 1.0f;
	glm::ivec2 m_windowSize = glm::ivec2(1);
	GLFWwindow* m_window = nullptr;
	MouseInputMode m_mouseInputMode = MouseInputMode::NORMAL;
	SoundManager m_soundManager;
	AssetManager m_assetManager;
	MessageManager m_messageManager;
	PreferenceState	m_preferenceState;
	ActionState	m_actionState;
	InputBinding m_inputBindings;
	std::vector<std::pair<std::thread, std::promise<void>>> m_threads;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);


	// Private Modules
	ECS_Module m_moduleECS;
	StartScreen_Module m_moduleStartScreen;
	Game_Module m_moduleGame;
	LevelEditor_Module m_moduleEditor;
	Graphics_Module m_moduleGraphics;
	UI_Module m_moduleUI;
	Physics_Module m_modulePhysics;
};

#endif // ENGINE_H
