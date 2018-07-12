#include "Engine.h"
#include "Systems\World\Camera.h"
#include "Systems\System_Interface.h"
#include <direct.h>
#include <iostream>
#include <string>

// OpenGL Dependent Systems //
#include "GL\glew.h"
#include "GLFW\glfw3.h"

// Importers used //
#include "Utilities\IO\Image_IO.h"
#include "Utilities\IO\Model_IO.h"


static bool				m_Initialized_Sharing = false;
static GLFWwindow	*	m_Context_Sharing = nullptr;

static void GLFW_Callback_Error(int error, const char * description)
{
	std::cout << std::string("Unhandled GLFW Error (" + std::to_string(error) + "): " + description);
}

// Is called when the window resizes
static void GLFW_Callback_Windowresize(GLFWwindow * window, int width, int height)
{
	Engine & engine = *((Engine*)glfwGetWindowUserPointer(window));
	engine.getCamera()->setDimensions(glm::vec2(width, height));
	engine.getCamera()->update();
	engine.setPreference(PreferenceState::C_WINDOW_WIDTH, width);
	engine.setPreference(PreferenceState::C_WINDOW_HEIGHT, height);
}

Engine::~Engine()
{
	if (m_Initialized) {
		removePrefCallback(PreferenceState::C_SHADOW_QUALITY, this);
	}
}

Engine::Engine() : m_AssetManager(this), m_PreferenceState(this)
{
	m_Initialized = false;	
	m_lastTime = 0;
}

bool Initialize_Sharing(Engine * engine)
{
	if (!m_Initialized_Sharing) {

		engine->reportMessage("Starting Initialization...");
		glfwSetErrorCallback(GLFW_Callback_Error);
		if (!glfwInit()) {
			glfwTerminate();
			engine->reportError(MessageManager::OTHER_ERROR, "GLFW unable to initialize, shutting down...");
			return false;
		}

		// Create an invisible window for asset sharing
		const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		glfwWindowHint(GLFW_RED_BITS, mainMode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mainMode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mainMode->blueBits);
		glfwWindowHint(GLFW_ALPHA_BITS, 0);
		glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, DESIRED_OGL_VER_MAJOR);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, DESIRED_OGL_VER_MINOR);
		glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_NO_RESET_NOTIFICATION);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		m_Context_Sharing = glfwCreateWindow(512, 512, "", NULL, NULL);
		glfwMakeContextCurrent(m_Context_Sharing);
		glewExperimental = GL_TRUE;		
		if (glewInit() != GLEW_OK) {
			glfwTerminate();
			engine->reportError(MessageManager::OTHER_ERROR, "GLEW unable to initialize, shutting down...");
			return false;
		}
		engine->reportMessage("...success!\n");

		engine->reportMessage("**************************************************");
		engine->reportMessage("Engine Version: " + std::string(ENGINE_VERSION));
		engine->reportMessage("ASSIMP Version: " + Model_IO::Get_Version());
		engine->reportMessage("Bullet Version: N/A");
		engine->reportMessage("FreeImage Version: " + Image_IO::Get_Version());
		engine->reportMessage("GLEW Version: " + std::string(reinterpret_cast<char const *>(glewGetString(GLEW_VERSION))));
		engine->reportMessage("GLFW Version: " + std::string(glfwGetVersionString()));
		engine->reportMessage("GLM Version: " + std::to_string(GLM_VERSION_MAJOR) + "." + std::to_string(GLM_VERSION_MINOR) + "." + std::to_string(GLM_VERSION_PATCH) + "." + std::to_string(GLM_VERSION_REVISION));
		engine->reportMessage("OpenGL Version: " + std::string(reinterpret_cast<char const *>(glGetString(GL_VERSION))));
		engine->reportMessage("GLSL Version: " + std::string(reinterpret_cast<char const *>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
		engine->reportMessage("GL implementation provided by: " + std::string(reinterpret_cast<char const *>(glGetString(GL_VENDOR))));
		engine->reportMessage("Using GPU: " + std::string(reinterpret_cast<char const *>(glGetString(GL_RENDERER))));
		engine->reportMessage("**************************************************");
		
		m_Initialized_Sharing = true;
	}
	return m_Initialized_Sharing;
}

#include "Assets\Asset_Material.h"
#include "Systems\Preferences\Preferences.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\Input\Input.h"
#include "Systems\Logic\Logic.h"
#include "Systems\World\World.h"
#include "Systems\PerfCounter\PerfCounter.h"
bool Engine::initialize()
{
	if ((!m_Initialized) && Initialize_Sharing(this)) {
		m_Camera = new Camera();
		m_Camera->setHorizontalFOV(110.0f);
		const float farPlane = addPrefCallback(PreferenceState::C_SHADOW_QUALITY, this, [&](const float &f) { m_Camera->setFarPlane(f); m_Camera->update(); });
		//m_Camera->setNearPlane(1.0f);
		m_Camera->setFarPlane(farPlane);
		m_Camera->update();
		const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		glfwWindowHint(GLFW_RED_BITS, mainMode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mainMode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mainMode->blueBits);
		glfwWindowHint(GLFW_ALPHA_BITS, 0);
		glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, DESIRED_OGL_VER_MAJOR);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, DESIRED_OGL_VER_MINOR);
		glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_NO_RESET_NOTIFICATION);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		m_Context_Rendering = glfwCreateWindow(1, 1, "reVision", NULL, m_Context_Sharing); 
		glfwSetWindowIcon(m_Context_Rendering, 0, NULL);
		glfwMakeContextCurrent(m_Context_Rendering);	
				
		// Create all the required systems
		m_Systems["Preferences"] = new System_Preferences("preferences");
		m_Systems["Graphics"] = new System_Graphics();
		m_Systems["PerfCounter"] = new System_PerfCounter();
		m_Systems["Input"] = new System_Input(this);
		m_Systems["Logic"] = new System_Logic();
		m_Systems["World"] = new System_World();

		// Initialize all systems
		for each (auto &system in m_Systems)
			system.second->initialize(this);

		const float window_width = getPreference(PreferenceState::C_WINDOW_WIDTH);
		const float window_height = getPreference(PreferenceState::C_WINDOW_HEIGHT);
		const int maxWidth = mainMode->width, maxHeight = mainMode->height;
		glfwSetWindowSize(m_Context_Rendering, window_width, window_height);
		glfwSetWindowPos(m_Context_Rendering, (maxWidth - window_width) / 2, (maxHeight - window_height) / 2);
		glfwSetInputMode(m_Context_Rendering, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
		glfwSetCursorPos(m_Context_Rendering, 0, 0);
				
		glfwSetWindowUserPointer(m_Context_Rendering, this);
		glfwSetWindowSizeCallback(m_Context_Rendering, GLFW_Callback_Windowresize);		
		glfwSwapInterval(0);

		m_modelManager.initialize();
		m_materialManager.initialize();

		m_Initialized = true;
		m_frameCount = 0;
		m_frameAccumulator = 0;
	}
	return m_Initialized;
}

void Engine::shutdown()
{
	if (m_Initialized) {
		removePrefCallback(PreferenceState::C_DRAW_DISTANCE, this);
		
		for each (auto system in m_Systems)
			delete system.second;
		m_Systems.clear();

		m_Initialized = false;
	}
}

void Engine::tick()
{
 	float deltaTime = 0;
	float thisTime = glfwGetTime();
	if (m_Initialized && !glfwWindowShouldClose(m_Context_Rendering)) {
		deltaTime = thisTime - m_lastTime;
		m_lastTime = thisTime;

		// performance heuristics
		m_frameCount++;
		if (m_frameCount >= 100) {
			m_frameAccumulator /= 100.0f;
			reportMessage("Avg Frametime = " + std::to_string(m_frameAccumulator*1000.0f) + " ms");
			m_frameAccumulator = deltaTime;
			m_frameCount = 0;
		}
		else 
			m_frameAccumulator += deltaTime;		
		// end performance heuristics

		// Tick managers
		m_AssetManager.notifyObservers();
		m_modelManager.update();
		m_materialManager.parseWorkOrders();

		// Tick Systems
		for each (auto system in m_Systems) 			
			system.second->update(deltaTime);		
		
		// End Frame
		glfwSwapBuffers(m_Context_Rendering);
		glfwPollEvents();
	}
}

void Engine::tickThreaded()
{
	glfwMakeContextCurrent(m_Context_Sharing);
	float lastTime = 0, thisTime = 0, deltaTime = 0;
	bool stay_alive = true;
	while (stay_alive) {
		if (m_Initialized && m_Initialized_Sharing) {
			thisTime = glfwGetTime();
			deltaTime = thisTime - lastTime;
			lastTime = thisTime;
			m_AssetManager.finalizeOrders();
			for each (auto system in m_Systems)
				system.second->updateThreaded(deltaTime);
		}
		stay_alive = !shouldClose();
	}
}

bool Engine::shouldClose()
{
	return glfwWindowShouldClose(m_Context_Rendering);
}

void Engine::reportMessage(const std::string & input)
{
	m_messageManager.statement(input);
}

void Engine::reportError(const int & error_number, const std::string & input, const std::string & additional_input)
{
	m_messageManager.error(error_number, input, additional_input);
}

std::string Engine::Get_Current_Dir()
{
	// Technique to return the running directory of the application
	char cCurrentPath[FILENAME_MAX];
	if (_getcwd(cCurrentPath, sizeof(cCurrentPath)))
		cCurrentPath[sizeof(cCurrentPath) - 1] = '/0';
	return std::string(cCurrentPath);
}

bool Engine::File_Exists(const std::string & name)
{
	// Technique to return whether or not a given file or folder exists
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

