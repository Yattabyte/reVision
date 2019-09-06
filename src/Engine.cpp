#include "Engine.h"
#include "LinearMath/btScalar.h"
#include <direct.h>

// OpenGL Dependent Systems //
#include "Utilities/GL/glad/glad.h"
#include "GLFW/glfw3.h"

// Importers Used //
#include "Utilities/IO/Image_IO.h"
#include "Utilities/IO/Mesh_IO.h"

constexpr int DESIRED_OGL_VER_MAJOR = 4;
constexpr int DESIRED_OGL_VER_MINOR = 5;


Engine::~Engine()
{
	// Update indicator
	*m_aliveIndicator = false;
	m_moduleWorld.deinitialize();
	m_moduleGraphics.deinitialize();
	m_moduleUI.deinitialize();
	m_modulePhysics.deinitialize();
	m_moduleStartScreen.deinitialize();
	m_moduleEditor.deinitialize();
	m_moduleGame.deinitialize();
	Image_IO::Deinitialize();
	glfwDestroyWindow(m_window);
	glfwTerminate();
	m_messageManager.statement("Shutting down...");
}

Engine::Engine() :
	// Initialize engine-dependent members first
	m_inputBindings(this),
	m_preferenceState(this)
{
	// Initialize aN OGL context
	initWindow();
	Image_IO::Initialize();
	m_inputBindings.loadFile("binds");

	printBoilerPlate();
	m_moduleWorld.initialize(this);
	m_moduleGraphics.initialize(this);
	m_moduleUI.initialize(this);
	m_modulePhysics.initialize(this);
	m_moduleStartScreen.initialize(this);
	m_moduleEditor.initialize(this);
	m_moduleGame.initialize(this);

	initThreads();
	goToMainMenu();
}

void Engine::initWindow()
{
	// Initialize GLFW
	if (!glfwInit()) {
		m_messageManager.error("GLFW unable to initialize, shutting down...");
		glfwTerminate();
		exit(-1);
	}

	// Create main window
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
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
#ifdef DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
	m_window = glfwCreateWindow(1, 1, "", NULL, NULL);
	glfwMakeContextCurrent(m_window);
	glfwSetWindowIcon(m_window, 0, NULL);
	glfwSetCursorPos(m_window, 0, 0);

	// Initialize GLAD
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		m_messageManager.error("GLAD unable to initialize, shutting down...");
		glfwTerminate();
		exit(-1);
	}

	// Preference Values
	m_refreshRate = float(glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);
	m_windowSize.x = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
	m_windowSize.y = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;
	m_preferenceState.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_windowSize.x);
	m_preferenceState.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_windowSize.y);
	m_preferenceState.getOrSetValue(PreferenceState::C_WINDOW_REFRESH_RATE, m_refreshRate);
	m_preferenceState.getOrSetValue(PreferenceState::C_WINDOW_FULLSCREEN, m_useFullscreen);
	m_preferenceState.getOrSetValue(PreferenceState::C_VSYNC, m_vsync);

	// Preference Callbacks
	m_preferenceState.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {
		m_windowSize.x = int(f);
		configureWindow();
	});
	m_preferenceState.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {
		m_windowSize.y = int(f);
		configureWindow();
	});
	m_preferenceState.addCallback(PreferenceState::C_WINDOW_REFRESH_RATE, m_aliveIndicator, [&](const float &f) {
		m_refreshRate = f;
		configureWindow();
	});
	m_preferenceState.addCallback(PreferenceState::C_WINDOW_FULLSCREEN, m_aliveIndicator, [&](const float &f) {
		m_useFullscreen = f;
		configureWindow();
	});
	m_preferenceState.addCallback(PreferenceState::C_VSYNC, m_aliveIndicator, [&](const float &f) {
		m_vsync = f;
		glfwSwapInterval((int)f);
	});
	configureWindow();
	glfwSwapInterval((int)m_vsync);
	glfwSetInputMode(m_window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
	glfwSetWindowUserPointer(m_window, this);
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow * window, int width, int height) {
		auto & preferences = ((Engine*)glfwGetWindowUserPointer(window))->getPreferenceState();
		preferences.setValue(PreferenceState::C_WINDOW_WIDTH, width);
		preferences.setValue(PreferenceState::C_WINDOW_HEIGHT, height);
	});
	glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) {
		((Engine*)glfwGetWindowUserPointer(window))->getModule_UI().applyCursorPos(xPos, yPos);
	});
	glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
		((Engine*)glfwGetWindowUserPointer(window))->getModule_UI().applyCursorButton(button, action, mods);
	});
	glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int character) {
		((Engine*)glfwGetWindowUserPointer(window))->getModule_UI().applyChar(character);
	});
	glfwSetKeyCallback(m_window, [](GLFWwindow* window, int a, int b, int c, int d) {
		((Engine*)glfwGetWindowUserPointer(window))->getModule_UI().applyKey(a, b, c, d);
	});
#ifdef DEBUG
	if (GLAD_GL_KHR_debug) {
		GLint v;
		glGetIntegerv(GL_CONTEXT_FLAGS, &v);
		if (v && GL_CONTEXT_FLAG_DEBUG_BIT) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			m_messageManager.statement(">>> KHR DEBUG MODE ENABLED <<<");
			auto myCallback = [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const void *data) {
				char* _source;
				char* _type;
				char* _severity;
				switch (source) {
				case GL_DEBUG_SOURCE_API:
					_source = "API";
					break;

				case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
					_source = "WINDOW SYSTEM";
					break;

				case GL_DEBUG_SOURCE_SHADER_COMPILER:
					_source = "SHADER COMPILER";
					break;

				case GL_DEBUG_SOURCE_THIRD_PARTY:
					_source = "THIRD PARTY";
					break;

				case GL_DEBUG_SOURCE_APPLICATION:
					_source = "APPLICATION";
					break;

				case GL_DEBUG_SOURCE_OTHER:
					_source = "UNKNOWN";
					break;

				default:
					_source = "UNKNOWN";
					break;
				}

				switch (type) {
				case GL_DEBUG_TYPE_ERROR:
					_type = "ERROR";
					break;

				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
					_type = "DEPRECATED BEHAVIOR";
					break;

				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
					_type = "UDEFINED BEHAVIOR";
					break;

				case GL_DEBUG_TYPE_PORTABILITY:
					_type = "PORTABILITY";
					break;

				case GL_DEBUG_TYPE_PERFORMANCE:
					_type = "PERFORMANCE";
					break;

				case GL_DEBUG_TYPE_OTHER:
					_type = "OTHER";
					break;

				case GL_DEBUG_TYPE_MARKER:
					_type = "MARKER";
					break;

				default:
					_type = "UNKNOWN";
					break;
				}

				switch (severity) {
				case GL_DEBUG_SEVERITY_HIGH:
					_severity = "HIGH";
					break;

				case GL_DEBUG_SEVERITY_MEDIUM:
					_severity = "MEDIUM";
					break;

				case GL_DEBUG_SEVERITY_LOW:
					_severity = "LOW";
					break;

				case GL_DEBUG_SEVERITY_NOTIFICATION:
					_severity = "NOTIFICATION";
					break;

				default:
					_severity = "UNKNOWN";
					break;
				}

				if (severity != GL_DEBUG_SEVERITY_NOTIFICATION && severity != GL_DEBUG_SEVERITY_LOW)
					((MessageManager*)(data))->error(
						std::to_string(id) + ": " + std::string(_type) + " of " + std::string(_severity) + " severity, raised from " + std::string(_source) + ": " + std::string(msg, length));
			};
			glDebugMessageCallbackKHR(myCallback, &m_messageManager);
		}
	}
#endif
}

void Engine::initThreads()
{
	const unsigned int maxThreads = std::max(1u, std::thread::hardware_concurrency());
	for (unsigned int x = 0; x < maxThreads; ++x) {
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
		std::promise<void> exitSignal;
		std::future<void> exitObject = exitSignal.get_future();
		std::thread workerThread(&Engine::tickThreaded, this, std::move(exitObject), std::move(glfwCreateWindow(1, 1, "", NULL, m_window)));
		workerThread.detach();
		m_threads.push_back(std::move(std::make_pair(std::move(workerThread), std::move(exitSignal))));
	}
}

void Engine::printBoilerPlate()
{
	m_messageManager.statement("*****************************************");
	m_messageManager.statement("* > reVision Engine:\t\t\t*");
#ifdef NDEBUG
	m_messageManager.statement("*  - Version      " + std::string(ENGINE_VERSION) + " (RELEASE)\t*");
#else
	m_messageManager.statement("*  - Version      " + std::string(ENGINE_VERSION) + " (DEBUG)\t\t*");
#endif // DEBUG
	m_messageManager.statement("*  - Build Date   September 6th, 2019\t*");
	m_messageManager.statement("*****************************************");
	m_messageManager.statement("* > Library Info:\t\t\t*");
	m_messageManager.statement("*  - ASSIMP       " + Mesh_IO::Get_Version() + "\t\t*");
	m_messageManager.statement("*  - Bullet       " + std::to_string(BT_BULLET_VERSION) + "\t\t\t*");
	m_messageManager.statement("*  - FreeImage    " + Image_IO::Get_Version() + "\t\t*");
	m_messageManager.statement("*  - GLAD         " + std::to_string(GLVersion.major) + "." + std::to_string(GLVersion.minor) + "\t\t\t*");
	m_messageManager.statement("*  - GLFW         " + std::string(glfwGetVersionString(), 5) + "\t\t\t*");
	m_messageManager.statement("*  - GLM          " + std::to_string(GLM_VERSION_MAJOR) + "." + std::to_string(GLM_VERSION_MINOR) + "." + std::to_string(GLM_VERSION_PATCH) + "." + std::to_string(GLM_VERSION_REVISION) + "\t\t*");;
	m_messageManager.statement("*  - SoLoud       " + std::to_string(m_soundManager.GetVersion()) + "\t\t*");
	m_messageManager.statement("*****************************************");
	m_messageManager.statement("* > Graphics Info:\t\t\t*");
	m_messageManager.statement("*  - " + std::string(reinterpret_cast<char const *>(glGetString(GL_RENDERER))) + "\t\t*");
	m_messageManager.statement("*  - OpenGL " + std::string(reinterpret_cast<char const *>(glGetString(GL_VERSION))) + "\t\t*");
	m_messageManager.statement("*  - GLSL " + std::string(reinterpret_cast<char const *>(glGetString(GL_SHADING_LANGUAGE_VERSION))) + "\t\t\t*");
	m_messageManager.statement("*****************************************");
}

void Engine::tick()
{
	///*----------------------------------------------------BEGIN FRAME----------------------------------------------------*///
	const float thisTime = (float)glfwGetTime(), deltaTime = thisTime - m_lastTime;
	m_lastTime = thisTime;

	// Update Managers
	m_assetManager.notifyObservers();
	
	// Updated mouse states, manually
	double mouseX, mouseY;
	glfwGetCursorPos(m_window, &mouseX, &mouseY);
	m_actionState[ActionState::MOUSE_L] = (float)glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT);
	m_actionState[ActionState::MOUSE_R] = (float)glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT);
	m_actionState[ActionState::MOUSE_X] = (float)mouseX;
	m_actionState[ActionState::MOUSE_Y] = (float)mouseY;
	if (m_mouseInputMode == FREE_LOOK)
		glfwSetCursorPos(m_window, 0, 0);	

	// Update key binding states, manually
	if (const auto &bindings = m_inputBindings.getBindings())
		if (bindings->existsYet())
			for each (const auto &pair in bindings.get()->m_configuration)
				m_actionState[pair.first] = glfwGetKey(m_window, (int)pair.second) ? 1.0f : 0.0f;

	// Update UI module based on action state, manually
	m_moduleUI.applyActionState(m_actionState);

	// Tick relevant systems
	m_modulePhysics.frameTick(deltaTime);
	m_moduleWorld.frameTick(deltaTime);
	if (m_engineState == in_startMenu)
		m_moduleStartScreen.frameTick(deltaTime);
	else if (m_engineState == in_game)
		m_moduleGame.frameTick(deltaTime);
	m_moduleGraphics.frameTick(deltaTime);
	if (m_engineState == in_editor)
		m_moduleEditor.frameTick(deltaTime);
	m_moduleUI.frameTick(deltaTime);	

	// This is done last so they can appear over-top
	m_moduleGame.renderOverlays(deltaTime); 

	// Swap buffers and end
	glfwSwapBuffers(m_window);
	glfwPollEvents();
	///*-----------------------------------------------------END FRAME-----------------------------------------------------*///
}

void Engine::tickThreaded(std::future<void> exitObject, GLFWwindow * const window)
{
	glfwMakeContextCurrent(window);

	// Check if thread should shutdown
	while (exitObject.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout)
		m_assetManager.beginWorkOrder();
}

bool Engine::shouldClose()
{
	return glfwWindowShouldClose(m_window);
}

void Engine::shutDown()
{
	glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

void Engine::setMouseInputMode(const MouseInputMode & mode)
{
	m_mouseInputMode = mode;
	switch (mode) {
	case NORMAL:
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	case FREE_LOOK:
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPos(m_window, m_actionState[ActionState::LOOK_X], m_actionState[ActionState::LOOK_Y]);
		m_actionState[ActionState::LOOK_X] = m_actionState[ActionState::MOUSE_X];
		m_actionState[ActionState::LOOK_Y] = m_actionState[ActionState::MOUSE_Y];
		break;
	}
}

void Engine::goToMainMenu()
{
	m_engineState = in_startMenu;
	m_moduleStartScreen.showStartMenu();
}

void Engine::goToGame()
{
	m_engineState = in_game;
	m_moduleGame.showGame();
}

void Engine::goToEditor()
{
	m_engineState = in_editor;
	m_moduleEditor.showEditor();
}

float Engine::getTime() const
{
	return (float)glfwGetTime();
}

std::vector<glm::ivec3> Engine::getResolutions() const
{
	int count(0);
	const GLFWvidmode* modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
	std::vector<glm::ivec3> resolutions(count);
	for (int x = 0; x < count; ++x)
		resolutions[x] = { modes[x].width, modes[x].height, modes[x].refreshRate };
	return resolutions;
}

std::string Engine::Get_Current_Dir()
{
	// Technique to return the running directory of the application
	char cCurrentPath[FILENAME_MAX];
	if (_getcwd(cCurrentPath, sizeof(cCurrentPath)))
		cCurrentPath[sizeof(cCurrentPath) - 1ull] = char('\0');
	return std::string(cCurrentPath);
}

bool Engine::File_Exists(const std::string & name)
{
	// Technique to return whether or not a given file or folder exists
	struct stat buffer;
	return (stat((Engine::Get_Current_Dir() + name).c_str(), &buffer) == 0);
}

void Engine::configureWindow()
{
	const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	const int maxWidth = mainMode->width, maxHeight = mainMode->height;
	glfwSetWindowSize(m_window, m_windowSize.x, m_windowSize.y);
	glfwSetWindowPos(m_window, (maxWidth - m_windowSize.x) / 2, (maxHeight - m_windowSize.y) / 2);
	glfwSetWindowMonitor(
		m_window,
		m_useFullscreen ? glfwGetPrimaryMonitor() : NULL,
		0, 0,
		m_windowSize.x, m_windowSize.y,
		(int)m_refreshRate
	);
}
