#include "Engine.h"
#include "LinearMath\btScalar.h"
#include <direct.h>

// OpenGL Dependent Systems //
#include "GL\glew.h"
#include "GLFW\glfw3.h"

// Importers Used //
#include "Utilities\IO\Image_IO.h"
#include "Utilities\IO\Mesh_IO.h"

// General Logical ECS Systems Used //
#include "ECS\Systems\PlayerMovement_S.h"
#include "ECS\Systems\SkeletonAnimation_S.h"
#include "ECS\Systems\TransformSync_S.h"


// Is called when the window resizes
static void GLFW_Callback_Windowresize(GLFWwindow * window, int width, int height)
{
	auto & preferences = ((Engine*)glfwGetWindowUserPointer(window))->getPreferenceState();
	preferences.setValue(PreferenceState::C_WINDOW_WIDTH, width);
	preferences.setValue(PreferenceState::C_WINDOW_HEIGHT, height);
}

Rendering_Context::~Rendering_Context()
{
	glfwDestroyWindow(window);
}

Rendering_Context::Rendering_Context(Engine * engine)
{
	// Begin Initialization
	engine->reportMessage("Creating Window...");
	if (!glfwInit()) {
		engine->reportError(MessageManager::MANUAL_ERROR, "GLFW unable to initialize, shutting down...");
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, 0, 0);
	glfwSwapInterval(vsync);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		engine->reportError(MessageManager::MANUAL_ERROR, "GLEW unable to initialize, shutting down...");
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
	reportMessage("Shutting down...");
	Image_IO::Deinitialize();	
	glfwTerminate();
}

Engine::Engine() : 
	// Initialize engine-dependent members first
	m_AssetManager(this), 
	m_inputBindings(this), 
	m_PreferenceState(this),
	m_renderingContext(this), 
	m_materialManager(this), 
	m_moduleGraphics(this), 
	m_modulePhysics(this), 
	m_moduleWorld(this)
{
	Image_IO::Initialize();

	// Preference Values
	m_PreferenceState.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_windowSize.x);
	m_PreferenceState.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_windowSize.y);
	m_PreferenceState.getOrSetValue(PreferenceState::C_WINDOW_REFRESH_RATE, m_refreshRate);

	// Preference Callbacks
	m_PreferenceState.addCallback(PreferenceState::C_WINDOW_USE_MONITOR_RATE, m_aliveIndicator, [&](const float &f) {
		if (f > 0.0f) 
			glfwSetWindowMonitor(m_renderingContext.window, glfwGetPrimaryMonitor(), 0, 0, m_windowSize.x, m_windowSize.y, glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);		
		else
			glfwSetWindowMonitor(m_renderingContext.window, glfwGetPrimaryMonitor(), 0, 0, m_windowSize.x, m_windowSize.y, m_refreshRate > 0.0f ? (int)m_refreshRate : GLFW_DONT_CARE);
	});
	m_PreferenceState.addCallback(PreferenceState::C_WINDOW_REFRESH_RATE, m_aliveIndicator, [&](const float &f) {
		m_refreshRate = f;
		int useMonitorRate = 1;
		m_PreferenceState.getOrSetValue(PreferenceState::C_WINDOW_USE_MONITOR_RATE, useMonitorRate);
		if (useMonitorRate > 0)
			glfwSetWindowMonitor(m_renderingContext.window, glfwGetPrimaryMonitor(), 0, 0, m_windowSize.x, m_windowSize.y, glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);		
		else
			glfwSetWindowMonitor(m_renderingContext.window, glfwGetPrimaryMonitor(), 0, 0, m_windowSize.x, m_windowSize.y, m_refreshRate > 0.0f ? (int)m_refreshRate : GLFW_DONT_CARE);
	});
	m_PreferenceState.addCallback(PreferenceState::C_VSYNC, m_aliveIndicator, [&](const float &f) {glfwSwapInterval((int)f);});

	// Configure Rendering Context
	const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	const int maxWidth = mainMode->width, maxHeight = mainMode->height;
	glfwSetWindowSize(m_renderingContext.window, m_windowSize.x, m_windowSize.y);
	glfwSetWindowPos(m_renderingContext.window, (maxWidth - m_windowSize.x) / 2, (maxHeight - m_windowSize.y) / 2);
	glfwSetWindowUserPointer(m_renderingContext.window, this);
	glfwSetWindowSizeCallback(m_renderingContext.window, GLFW_Callback_Windowresize);
	glfwMakeContextCurrent(m_renderingContext.window);
	
	// Initialize Members
	m_inputBindings.loadFile("binds");
	m_moduleGraphics.initialize();
	m_modulePhysics.initialize();
	m_moduleWorld.initialize();
	m_modelManager.initialize();

	m_logicSystems.addSystem(new PlayerMovement_System(this));
	m_logicSystems.addSystem(new SkeletonAnimation_System());
	m_logicSystems.addSystem(new TransformSync_System(this, m_modulePhysics.getWorld()));

	const unsigned int maxThreads = std::max(1u, std::thread::hardware_concurrency());
	for (unsigned int x = 0; x < maxThreads; ++x) {
		std::promise<void> exitSignal;
		std::future<void> exitObject = exitSignal.get_future();
		std::thread workerThread(&Engine::tickThreaded, this, std::move(exitObject), std::move(Auxilliary_Context(m_renderingContext)));
		workerThread.detach();
		m_threads.push_back(std::move(std::make_pair(std::move(workerThread), std::move(exitSignal))));
	}

	reportMessage("**************************************************");
	reportMessage("Engine Version: " + std::string(ENGINE_VERSION));
	reportMessage("ASSIMP Version: " + Mesh_IO::Get_Version());
	reportMessage("Bullet Version: " + std::to_string(BT_BULLET_VERSION));
	reportMessage("FreeImage Version: " + Image_IO::Get_Version());
	reportMessage("GLEW Version: " + std::string(reinterpret_cast<char const *>(glewGetString(GLEW_VERSION))));
	reportMessage("GLFW Version: " + std::string(glfwGetVersionString()));
	reportMessage("GLM Version: " + std::to_string(GLM_VERSION_MAJOR) + "." + std::to_string(GLM_VERSION_MINOR) + "." + std::to_string(GLM_VERSION_PATCH) + "." + std::to_string(GLM_VERSION_REVISION));
	reportMessage("OpenGL Version: " + std::string(reinterpret_cast<char const *>(glGetString(GL_VERSION))));
	reportMessage("GLSL Version: " + std::string(reinterpret_cast<char const *>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
	reportMessage("GL implementation provided by: " + std::string(reinterpret_cast<char const *>(glGetString(GL_VENDOR))));
	reportMessage("Using GPU: " + std::string(reinterpret_cast<char const *>(glGetString(GL_RENDERER))));
	reportMessage("**************************************************");
}

void Engine::tick()
{
	float thisTime = (float)glfwGetTime();
	float deltaTime = thisTime - m_lastTime;
	m_lastTime = thisTime;

	// Performance Debugging
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

	// Logic depending on assets finalizing
	m_AssetManager.notifyObservers();
	// Update expandable model container
	m_modelManager.update();
	// Update input
	updateInput(deltaTime);

	// Logic depending on state of the world
	if (m_moduleWorld.checkIfLoaded()) {
		// Update physics
		m_modulePhysics.physicsFrame(deltaTime);
	}
	// Update logical systems
	m_ecs.updateSystems(m_logicSystems, deltaTime);
	// Update graphics
	m_moduleGraphics.setActiveCamera(0);
	m_moduleGraphics.renderFrame(deltaTime);

	// End Frame
	glfwSwapBuffers(m_renderingContext.window);
	glfwPollEvents();	
}

void Engine::tickThreaded(std::future<void> exitObject, const Auxilliary_Context && context)
{
	glfwMakeContextCurrent(context.window);

	// Check if thread should shutdown
	while (exitObject.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
		m_AssetManager.beginWorkOrder();
	}
}

bool Engine::shouldClose()
{
	return glfwWindowShouldClose(m_renderingContext.window);
}

void Engine::reportMessage(const std::string & input)
{
	m_messageManager.statement(input);
}

void Engine::reportError(const int & error_number, const std::string & input, const std::string & additional_input)
{
	m_messageManager.error(error_number, input, additional_input);
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
		cCurrentPath[sizeof(cCurrentPath) - 1] = char('/0');
	return std::string(cCurrentPath);
}

bool Engine::File_Exists(const std::string & name)
{
	// Technique to return whether or not a given file or folder exists
	struct stat buffer;
	return (stat((Engine::Get_Current_Dir() + name).c_str(), &buffer) == 0);
}

void Engine::updateInput(const float & deltaTime)
{
	const auto &bindings = m_inputBindings.getBindings();
	if (!bindings->existsYet()) return;
	const auto &bind_map = bindings.get()->m_configuration;
	// Pair is the action followed by the key assigned to it
	for each (const auto &pair in bind_map) {
		const auto &action = pair.first;
		const auto &input_button = (int)pair.second;
		// If Key is pressed, set state to 1, otherwise set to 0
		m_ActionState.at(action) = (glfwGetKey(m_renderingContext.window, input_button)) ? 1.0f : 0.0f;
	}
	double mouseX, mouseY;
	glfwGetCursorPos(m_renderingContext.window, &mouseX, &mouseY);
	m_ActionState.at(ActionState::LOOK_X) = (float)mouseX;
	m_ActionState.at(ActionState::LOOK_Y) = (float)mouseY;
	glfwSetCursorPos(m_renderingContext.window, 0, 0);
}