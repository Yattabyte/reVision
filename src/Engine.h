#pragma once
#ifndef ENGINE_H
#define ENGINE_H

#include "Utilities\ECS\ecs.h"
#include "Managers\AssetManager.h"
#include "Managers\ModelManager.h"
#include "Managers\MaterialManager.h"
#include "Managers\MessageManager.h"
#include "Modules\Graphics\Graphics_M.h"
#include "Modules\Physics\Physics_M.h"
#include "Modules\World\World_M.h"
#include "Modules\Game\Game_M.h"
#include "Utilities\ActionState.h"
#include "Utilities\InputBinding.h"
#include "Utilities\PreferenceState.h"
#include "Utilities\MappedChar.h"
#include <string>


constexpr char ENGINE_VERSION[] = "2.1.D";
constexpr int DESIRED_OGL_VER_MAJOR = 4;
constexpr int DESIRED_OGL_VER_MINOR = 5;

struct GLFWwindow;
class Engine;
struct Rendering_Context {
	~Rendering_Context();
	Rendering_Context(Engine * engine);
	GLFWwindow * window = NULL;
};
struct Auxilliary_Context {
	Auxilliary_Context(const Rendering_Context & other);
	GLFWwindow * window = NULL;
};

/** The main game engine object. Encapsulates the entire engine state. */
class Engine {
public:
	// Constructors
	/** Destroys the engine. */
	~Engine();
	/** Zero-initialize the engine. */
	Engine();


	// Public Methods
	/** Ticks the engine's overall simulation by a frame from the main thread. */
	void tick();
	/** Ticks the engine's overall simulation by a frame from a secondary thread. 
	@param	exitObject	object signaling when to close the thread */
	void tickThreaded(std::future<void> exitObject, const Auxilliary_Context && context);
	/** Checks if the engine wants to shut down.
	@return	true if engine should shut down */
	bool shouldClose();
	/** Adds a component constructor to the ecs construction map.
	@param	name				the component name type.
	@param	constructor			the component constructor object. */
	inline void registerECSConstructor(const char * name, BaseECSComponentConstructor * constructor) {
		m_ecs.registerConstructor(name, constructor);
	}


	// Public Accessors
	/** Returns this engine's rendering context. */
	GLFWwindow * getRenderingContext() const;
	/** Returns this engine's entity component system. */
	ECS & getECS() { return m_ecs; }
	/** Returns this engine's action state. */
	ActionState & getActionState() { return m_actionState; }
	/** Returns this engine's preference state. */
	PreferenceState & getPreferenceState() { return m_preferenceState; }
	/** Returns this engine's asset manager. */
	AssetManager & getAssetManager() { return m_assetManager; }
	/** Returns this engine's model manager. */
	ModelManager & getModelManager() { return m_modelManager; }
	/** Returns this engine's material manager. */
	MaterialManager & getMaterialManager() { return m_materialManager; }
	/** Returns this engine's message manager. */
	MessageManager & getMessageManager() { return m_messageManager; }
	/** Returns this engine's graphics module. */
	Graphics_Module & getGraphicsModule() { return m_moduleGraphics; }
	/** Returns this engine's physics module. */
	Physics_Module & getPhysicsModule() { return m_modulePhysics; }
	/** Returns this engine's world module. */
	World_Module & getWorldModule() { return m_moduleWorld; }
	/** Returns this engine's game module. */
	Game_Module & getGameModule() { return m_moduleGame; }


	// Static Methods
	/** Retrieves the application's running directory.
	@return					std::string of the absolute directory that this executable ran from */
	static std::string Get_Current_Dir();
	/** Check if a given file exists, relative to the application directory.
	@param	name			the full file path
	@return					true if the file exists, false otherwise */
	static bool File_Exists(const std::string & name);

	
private:
	// Private Methods
	void updateInput(const float & deltaTime);


	// Private Attributes
	float m_lastTime = 0;
	float m_frameAccumulator = 0;
	int m_frameCount = 0;
	float m_refreshRate = 60.0f;
	glm::ivec2 m_windowSize = glm::ivec2(1);	
	AssetManager m_assetManager;
	MessageManager m_messageManager;
	PreferenceState	m_preferenceState;
	Rendering_Context m_renderingContext;
	MaterialManager m_materialManager;
	ModelManager m_modelManager;
	ECS	m_ecs;
	ActionState	m_actionState;
	InputBinding m_inputBindings;
	std::vector<std::pair<std::thread, std::promise<void>>> m_threads;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);


	// Private Modules
	Graphics_Module m_moduleGraphics;
	Physics_Module m_modulePhysics;
	World_Module m_moduleWorld;
	Game_Module m_moduleGame;
};

#endif // ENGINE_H