#include "Engine.h"
#include <direct.h>

// OpenGL Dependent Systems //
#include "GL\glew.h"
#include "GLFW\glfw3.h"

// Importers used //
#include "Utilities\IO\Image_IO.h"
#include "Utilities\IO\Model_IO.h"


// Is called when the window resizes
static void GLFW_Callback_Windowresize(GLFWwindow * window, int width, int height)
{
	Engine & engine = *((Engine*)glfwGetWindowUserPointer(window));
	engine.setPreference(PreferenceState::C_WINDOW_WIDTH, (float)width);
	engine.setPreference(PreferenceState::C_WINDOW_HEIGHT, (float)height);
}

Engine::~Engine()
{
	reportMessage("Shutting down...");
	removePrefCallback(PreferenceState::C_WINDOW_USE_MONITOR_RATE, this);
	removePrefCallback(PreferenceState::C_WINDOW_REFRESH_RATE, this);
	removePrefCallback(PreferenceState::C_VSYNC, this);
	Image_IO::Deinitialize();
	reportMessage("...done!");
}

Engine::Engine() : 
	// Initialize engine-dependent members first
	m_AssetManager(this), m_messageManager(), m_inputBindings(this), m_PreferenceState(this), m_renderingContext(this), m_materialManager(), m_moduleGraphics(this), m_moduleWorld(this)
{
	Image_IO::Initialize();
	// Default Parameters
	m_lastTime = 0;
	m_frameAccumulator = 0;
	m_frameCount = 0;

	// Configure Rendering Context
	const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	m_windowSize.x = (int)getPreference(PreferenceState::C_WINDOW_WIDTH);
	m_windowSize.y = (int)getPreference(PreferenceState::C_WINDOW_HEIGHT);
	const int maxWidth = mainMode->width, maxHeight = mainMode->height;
	glfwSetWindowSize(m_renderingContext.main, m_windowSize.x, m_windowSize.y);
	glfwSetWindowPos(m_renderingContext.main, (maxWidth - m_windowSize.x) / 2, (maxHeight - m_windowSize.y) / 2);
	glfwSetWindowUserPointer(m_renderingContext.main, this);
	glfwSetWindowSizeCallback(m_renderingContext.main, GLFW_Callback_Windowresize);
	glfwMakeContextCurrent(m_renderingContext.main);


	// Preference Callbacks
	addPrefCallback(PreferenceState::C_WINDOW_USE_MONITOR_RATE, this, [&](const float &f) {
		if (f > 0.0f) {
			const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(m_renderingContext.main, glfwGetPrimaryMonitor(), 0, 0, m_windowSize.x, m_windowSize.y, mainMode->refreshRate);
		}
		else 
			glfwSetWindowMonitor(m_renderingContext.main, glfwGetPrimaryMonitor(), 0, 0, m_windowSize.x, m_windowSize.y, m_refreshRate > 0.0f ? (int)m_refreshRate : GLFW_DONT_CARE);		
	});
	m_refreshRate = addPrefCallback(PreferenceState::C_WINDOW_REFRESH_RATE, this, [&](const float &f) {
		m_refreshRate = f;
		if (getPreference(PreferenceState::C_WINDOW_USE_MONITOR_RATE) > 0.0f) {
			const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(m_renderingContext.main, glfwGetPrimaryMonitor(), 0, 0, m_windowSize.x, m_windowSize.y, mainMode->refreshRate);
		}
		else
			glfwSetWindowMonitor(m_renderingContext.main, glfwGetPrimaryMonitor(), 0, 0, m_windowSize.x, m_windowSize.y, m_refreshRate > 0.0f ? (int)m_refreshRate : GLFW_DONT_CARE);
	});
	addPrefCallback(PreferenceState::C_VSYNC, this, [&](const float &f) {glfwSwapInterval((int)f); });

	// Initialize Members
	m_PreferenceState.loadFile("preferences");
	m_inputBindings.loadFile("binds");
	m_moduleGraphics.initialize();
	m_moduleWorld.initialize();
	m_modelManager.initialize();

	reportMessage("**************************************************");
	reportMessage("Engine Version: " + std::string(ENGINE_VERSION));
	reportMessage("ASSIMP Version: " + Model_IO::Get_Version());
	reportMessage("Bullet Version: N/A");
	reportMessage("FreeImage Version: " + Image_IO::Get_Version());
	reportMessage("GLEW Version: " + std::string(reinterpret_cast<char const *>(glewGetString(GLEW_VERSION))));
	reportMessage("GLFW Version: " + std::string(glfwGetVersionString()));
	reportMessage("GLM Version: " + std::to_string(GLM_VERSION_MAJOR) + "." + std::to_string(GLM_VERSION_MINOR) + "." + std::to_string(GLM_VERSION_PATCH) + "." + std::to_string(GLM_VERSION_REVISION));
	reportMessage("OpenGL Version: " + std::string(reinterpret_cast<char const *>(glGetString(GL_VERSION))));
	reportMessage("GLSL Version: " + std::string(reinterpret_cast<char const *>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
	reportMessage("GL implementation provided by: " + std::string(reinterpret_cast<char const *>(glGetString(GL_VENDOR))));
	reportMessage("Using GPU: " + std::string(reinterpret_cast<char const *>(glGetString(GL_RENDERER))));
	reportMessage("**************************************************");

	reportMessage("Loading World...");
	m_moduleWorld.loadWorld();
	reportMessage("...done!");
}

void Engine::tick()
{
	float thisTime = glfwGetTime();
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

	// Check world status
	m_moduleWorld.checkIfLoaded();

	// Tick managers
	m_AssetManager.notifyObservers();
	m_modelManager.update();
	m_materialManager.parseWorkOrders();
	m_materialManager.bind();

	// Tick Inpute
	updateInput(deltaTime);

	// Tick Graphics
	m_moduleGraphics.setActiveCamera(0);
	m_moduleGraphics.renderFrame(deltaTime);

	// End Frame
	glfwSwapBuffers(m_renderingContext.main);
	glfwPollEvents();	
}

void Engine::tickThreaded(std::future<void> exitObj)
{
	glfwMakeContextCurrent(m_renderingContext.shared);
	float lastTime = 0, thisTime = 0, deltaTime = 0;
	while (exitObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
		if (m_renderingContext.shared) {
			thisTime = glfwGetTime();
			deltaTime = thisTime - lastTime;
			lastTime = thisTime;
			m_AssetManager.finalizeOrders();
		}
	}
}

bool Engine::shouldClose()
{
	return glfwWindowShouldClose(m_renderingContext.main);
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
	return m_renderingContext.main; 
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

void Engine::updateInput(const float & deltaTime)
{
	const auto &bindings = m_inputBindings.getBindings();
	if (!bindings) return;
	const auto &bind_map = bindings.get()->m_configuration;
	// Pair is the action followed by the key assigned to it
	for each (const auto &pair in bind_map) {
		const auto &action = pair.first;
		const auto &input_button = (int)pair.second;
		// If Key is pressed, set state to 1, otherwise set to 0
		m_ActionState.at(action) = (glfwGetKey(m_renderingContext.main, input_button)) ? 1.0f : 0.0f;
	}
	double mouseX, mouseY;
	glfwGetCursorPos(m_renderingContext.main, &mouseX, &mouseY);
	m_ActionState.at(ActionState::LOOK_X) = mouseX;
	m_ActionState.at(ActionState::LOOK_Y) = mouseY;
	glfwSetCursorPos(m_renderingContext.main, 0, 0);
}

Rendering_Context::Rendering_Context(Engine * engine)
{
	engine->reportMessage("Initializing rendering context...");		
	if (!glfwInit()) {
		engine->reportError(MessageManager::MANUAL_ERROR, "GLFW unable to initialize, shutting down...");
		glfwTerminate();
		exit(-1);
	}

	// Create an invisible window for asset sharing
	const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_RED_BITS, mainMode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mainMode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mainMode->blueBits);
	glfwWindowHint(GLFW_ALPHA_BITS, 0);
	if (engine->getPreference(PreferenceState::C_WINDOW_USE_MONITOR_RATE) > 0.0F)
		glfwWindowHint(GLFW_REFRESH_RATE, mainMode->refreshRate);
	else {
		const float refreshRate = engine->getPreference(PreferenceState::C_WINDOW_REFRESH_RATE);
		glfwWindowHint(GLFW_REFRESH_RATE, refreshRate > 0.0f ? (int)refreshRate : GLFW_DONT_CARE);
	}
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
	shared = glfwCreateWindow(1, 1, "", NULL, NULL);
	glfwMakeContextCurrent(shared);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		engine->reportError(MessageManager::MANUAL_ERROR, "GLEW unable to initialize, shutting down...");
		glfwTerminate();
		exit(-1);
	}

	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	main = glfwCreateWindow(1, 1, "reVision", NULL, shared);
	glfwSetWindowIcon(main, 0, NULL);
	glfwMakeContextCurrent(main);
	glfwSetInputMode(main, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(main, 0, 0);
	glfwSwapInterval((int)engine->getPreference(PreferenceState::C_VSYNC));
	engine->reportMessage("...done!");
}
