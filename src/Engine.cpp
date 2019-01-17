#include "Engine.h"
#include "Utilities/ECS/Transform_C.h"
#include "LinearMath/btScalar.h"
#include <direct.h>

// OpenGL Dependent Systems //
#include "GL/glad/glad.h"
#include "GLFW/glfw3.h"

// Importers Used //
#include "Utilities/IO/Image_IO.h"
#include "Utilities/IO/Mesh_IO.h"


/***************************
----START GLFW CALLBACKS----
***************************/
static void GLFW_Callback_WindowResize(GLFWwindow * window, int width, int height)
{
	auto & preferences = ((Engine*)glfwGetWindowUserPointer(window))->getPreferenceState();
	preferences.setValue(PreferenceState::C_WINDOW_WIDTH, width);
	preferences.setValue(PreferenceState::C_WINDOW_HEIGHT, height);
}
static void GLFW_Callback_CursorPosition(GLFWwindow* window, double xPos, double yPos)
{
	((Engine*)glfwGetWindowUserPointer(window))->getModule_UI().applyCursorPos(xPos, yPos);
}
static void GLFW_Callback_CursorButton(GLFWwindow* window, int button, int action, int mods)
{
	((Engine*)glfwGetWindowUserPointer(window))->getModule_UI().applyCursorButton(button, action, mods);
}
/***************************
-----END GLFW CALLBACKS-----
***************************/

Rendering_Context::~Rendering_Context()
{
	glfwDestroyWindow(window);
}

Rendering_Context::Rendering_Context(Engine * engine)
{
	// Begin Initialization
	if (!glfwInit()) {
		engine->getManager_Messages().error("GLFW unable to initialize, shutting down...");
		glfwTerminate();
		exit(-1);
	}
	auto & preferences = engine->getPreferenceState();
	int useMonitorRate = 1, desiredRate = 60, vsync = 1;
	preferences.getOrSetValue(PreferenceState::C_WINDOW_USE_MONITOR_RATE, useMonitorRate);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_REFRESH_RATE, desiredRate);
	preferences.getOrSetValue(PreferenceState::C_VSYNC, vsync);

	// Create an invisible window for asset sharing
	const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_RED_BITS, mainMode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mainMode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mainMode->blueBits);
	glfwWindowHint(GLFW_ALPHA_BITS, 0);
	if (useMonitorRate > 0)
		glfwWindowHint(GLFW_REFRESH_RATE, mainMode->refreshRate);
	else 
		glfwWindowHint(GLFW_REFRESH_RATE, desiredRate > 0 ? desiredRate : GLFW_DONT_CARE);	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, DESIRED_OGL_VER_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, DESIRED_OGL_VER_MINOR);
	glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_NO_RESET_NOTIFICATION);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_AUTO_ICONIFY, GL_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
	window = glfwCreateWindow(1, 1, "", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowIcon(window, 0, NULL);
	glfwSetCursorPos(window, 0, 0);
	glfwSwapInterval(vsync);

	// Initialize GLAD
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		engine->getManager_Messages().error("GLAD unable to initialize, shutting down...");
		glfwTerminate();
		exit(-1);
	}
}

Auxilliary_Context::Auxilliary_Context(const Rendering_Context & otherContext)
{
	// Create an invisible window for multi-threaded GL operations
	const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_RED_BITS, mainMode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mainMode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mainMode->blueBits);
	glfwWindowHint(GLFW_ALPHA_BITS, 0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, DESIRED_OGL_VER_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, DESIRED_OGL_VER_MINOR);
	glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_NO_RESET_NOTIFICATION);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_AUTO_ICONIFY, GL_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	window = glfwCreateWindow(1, 1, "", NULL, otherContext.window);
}

Engine::~Engine()
{
	// Update indicator
	m_aliveIndicator = false;
	m_messageManager.statement("Shutting down...");
	Image_IO::Deinitialize();	
	glfwTerminate();
}

Engine::Engine() : 
	// Initialize engine-dependent members first
	m_soundManager(),
	m_assetManager(this), 
	m_inputBindings(this), 
	m_preferenceState(this),
	m_renderingContext(this), 
	m_materialManager(this)
{
	// Output engine boiler-plate
	m_messageManager.statement("**************************************************");
	m_messageManager.statement("Engine Version " + std::string(ENGINE_VERSION));
	m_messageManager.statement("");
	m_messageManager.statement("Library Info:");
	m_messageManager.statement("ASSIMP       " + Mesh_IO::Get_Version());
	m_messageManager.statement("Bullet       " + std::to_string(BT_BULLET_VERSION));
	m_messageManager.statement("FreeImage    " + Image_IO::Get_Version());
	m_messageManager.statement("GLAD         " + std::to_string(GLVersion.major) + "." + std::to_string(GLVersion.minor));
	m_messageManager.statement("GLFW         " + std::string(glfwGetVersionString(), 5));
	m_messageManager.statement("GLM          " + std::to_string(GLM_VERSION_MAJOR) + "." + std::to_string(GLM_VERSION_MINOR) + "." + std::to_string(GLM_VERSION_PATCH) + "." + std::to_string(GLM_VERSION_REVISION));
	m_messageManager.statement("SoLoud       " + std::to_string(m_soundManager.GetVersion()));
	m_messageManager.statement("");
	m_messageManager.statement("Graphics Info:");
	m_messageManager.statement(std::string(reinterpret_cast<char const *>(glGetString(GL_RENDERER))));
	m_messageManager.statement("OpenGL " + std::string(reinterpret_cast<char const *>(glGetString(GL_VERSION))));
	m_messageManager.statement("GLSL " + std::string(reinterpret_cast<char const *>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
	m_messageManager.statement("**************************************************");

	Image_IO::Initialize();

	// Preference Values
	m_preferenceState.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_windowSize.x);
	m_preferenceState.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_windowSize.y);
	m_preferenceState.getOrSetValue(PreferenceState::C_WINDOW_REFRESH_RATE, m_refreshRate);

	// Preference Callbacks
	m_preferenceState.addCallback(PreferenceState::C_WINDOW_USE_MONITOR_RATE, m_aliveIndicator, [&](const float &f) {
		if (f > 0.0f) 
			glfwSetWindowMonitor(m_renderingContext.window, glfwGetPrimaryMonitor(), 0, 0, m_windowSize.x, m_windowSize.y, glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);		
		else
			glfwSetWindowMonitor(m_renderingContext.window, glfwGetPrimaryMonitor(), 0, 0, m_windowSize.x, m_windowSize.y, m_refreshRate > 0.0f ? (int)m_refreshRate : GLFW_DONT_CARE);
	});
	m_preferenceState.addCallback(PreferenceState::C_WINDOW_REFRESH_RATE, m_aliveIndicator, [&](const float &f) {
		m_refreshRate = f;
		int useMonitorRate = 1;
		m_preferenceState.getOrSetValue(PreferenceState::C_WINDOW_USE_MONITOR_RATE, useMonitorRate);
		if (useMonitorRate > 0)
			glfwSetWindowMonitor(m_renderingContext.window, glfwGetPrimaryMonitor(), 0, 0, m_windowSize.x, m_windowSize.y, glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);		
		else
			glfwSetWindowMonitor(m_renderingContext.window, glfwGetPrimaryMonitor(), 0, 0, m_windowSize.x, m_windowSize.y, m_refreshRate > 0.0f ? (int)m_refreshRate : GLFW_DONT_CARE);
	});
	m_preferenceState.addCallback(PreferenceState::C_VSYNC, m_aliveIndicator, [&](const float &f) {glfwSwapInterval((int)f);});

	// Configure Rendering Context
	const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	const int maxWidth = mainMode->width, maxHeight = mainMode->height;
	glfwSetWindowSize(m_renderingContext.window, m_windowSize.x, m_windowSize.y);
	glfwSetWindowPos(m_renderingContext.window, (maxWidth - m_windowSize.x) / 2, (maxHeight - m_windowSize.y) / 2);
	glfwSetWindowUserPointer(m_renderingContext.window, this);
	glfwSetWindowSizeCallback(m_renderingContext.window, GLFW_Callback_WindowResize);
	glfwSetCursorPosCallback(m_renderingContext.window, GLFW_Callback_CursorPosition);
	glfwSetMouseButtonCallback(m_renderingContext.window, GLFW_Callback_CursorButton);
	glfwMakeContextCurrent(m_renderingContext.window);
	
	// Initialize Members
	registerECSConstructor("Transform_Component", new Transform_Constructor());
	m_inputBindings.loadFile("binds");
	m_modelManager.initialize();
	m_moduleGraphics.initialize(this);
	m_modulePProcess.initialize(this);
	m_moduleUI.initialize(this);
	m_modulePhysics.initialize(this);
	m_moduleWorld.initialize(this);
	m_moduleGame.initialize(this);

	const unsigned int maxThreads = std::max(1u, std::thread::hardware_concurrency());
	for (unsigned int x = 0; x < maxThreads; ++x) {
		std::promise<void> exitSignal;
		std::future<void> exitObject = exitSignal.get_future();
		std::thread workerThread(&Engine::tickThreaded, this, std::move(exitObject), std::move(Auxilliary_Context(m_renderingContext)));
		workerThread.detach();
		m_threads.push_back(std::move(std::make_pair(std::move(workerThread), std::move(exitSignal))));
	}	
}

void Engine::tick()
{
	const float thisTime = (float)glfwGetTime();
	const float deltaTime = thisTime - m_lastTime;
	m_lastTime = thisTime;

	// Performance Debugging
	m_frameCount++;
	/*if (m_frameCount >= 100) {
		m_frameAccumulator /= 100.0f;
		m_messageManager.statement("Avg Frametime = " + std::to_string(m_frameAccumulator * 1000.0f) + " ms");
		m_frameAccumulator = deltaTime;
		m_frameCount = 0;
	}
	else
		m_frameAccumulator += deltaTime;*/

	// Update Managers
	m_assetManager.notifyObservers();
	m_modelManager.update();
	updateInput();
	m_moduleGraphics.setActiveCamera(0);

	/*******************
	--- Tick Modules ---
	*******************/
	if (m_moduleWorld.checkIfLoaded()) 
		m_modulePhysics.frameTick(deltaTime);	
	m_moduleGraphics.frameTick(deltaTime);
	m_moduleGame.frameTick(deltaTime);
	m_modulePProcess.frameTick(deltaTime);
	m_moduleUI.frameTick(deltaTime);

	// End Frame
	glfwSwapBuffers(m_renderingContext.window);
	glfwPollEvents();	
}

void Engine::tickThreaded(std::future<void> exitObject, const Auxilliary_Context && context)
{
	glfwMakeContextCurrent(context.window);

	// Check if thread should shutdown
	while (exitObject.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) 
		m_assetManager.beginWorkOrder();	
}

bool Engine::shouldClose()
{
	return glfwWindowShouldClose(m_renderingContext.window);
}

void Engine::shutDown()
{
	glfwSetWindowShouldClose(m_renderingContext.window, GLFW_TRUE);
}

void Engine::registerECSConstructor(const char * name, BaseECSComponentConstructor * constructor) 
{
	m_ecs.registerConstructor(name, constructor);
}

GLFWwindow * Engine::getRenderingContext() const
{ 
	return m_renderingContext.window; 
}

std::string Engine::Get_Current_Dir()
{
	// Technique to return the running directory of the application
	char cCurrentPath[FILENAME_MAX];
	if (_getcwd(cCurrentPath, sizeof(cCurrentPath)))
		cCurrentPath[sizeof(cCurrentPath) - 1l] = char('/0');
	return std::string(cCurrentPath);
}

bool Engine::File_Exists(const std::string & name)
{
	// Technique to return whether or not a given file or folder exists
	struct stat buffer;
	return (stat((Engine::Get_Current_Dir() + name).c_str(), &buffer) == 0);
}

void Engine::updateInput()
{
	const auto &bindings = m_inputBindings.getBindings();
	if (!bindings->existsYet()) return;
	const auto &bind_map = bindings.get()->m_configuration;
	// Pair is the action followed by the key assigned to it
	for each (const auto &pair in bind_map) {
		const auto &action = pair.first;
		const auto &input_button = (int)pair.second;
		// If Key is pressed, set state to 1, otherwise set to 0
		m_actionState.at(action) = (glfwGetKey(m_renderingContext.window, input_button)) ? 1.0f : 0.0f;
	}
	
	// UI manager dictates mouse control
	if (m_moduleUI.isCursorActive()) 
		glfwSetInputMode(m_renderingContext.window,	GLFW_CURSOR, GLFW_CURSOR_NORMAL);	
	else {
		glfwSetInputMode(m_renderingContext.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		double mouseX, mouseY;
		glfwGetCursorPos(m_renderingContext.window, &mouseX, &mouseY);
		m_actionState.at(ActionState::LOOK_X) = (float)mouseX;
		m_actionState.at(ActionState::LOOK_Y) = (float)mouseY;
		glfwSetCursorPos(m_renderingContext.window, 0, 0);
	}
}