#include "dt_Engine.h"
#include "Rendering\Camera.h"
#include "Rendering\Scenes\Scene.h"
#include "Systems\System_Interface.h"
#include <string>

#include "Systems\Message_Manager.h"
#include "Systems\Input\Input.h"
#include "Systems\Graphics\Graphics_PBR.h"
#include "Systems\Visibility.h"
#include "Systems\Logic\Logic.h"
#include "Systems\Asset_Manager.h"

// To replace with abstract systems
#include "Systems\Config_Manager.h"
#include "Systems\Material_Manager.h"
#include "Systems\Shadowmap_Manager.h"
#include "Systems\World_Manager.h"

// OpenGL Dependent Systems //
#include "GL\glew.h"
#include "GLFW\glfw3.h"


static bool				m_Initialized_Sharing = false;
static GLFWwindow	*	m_Context_Sharing = nullptr;

static void GLFW_Callback_Error(int error, const char* description)
{
	MSG::Error(GLFW_ERROR, "(" + to_string(error) + "): " + description);
}

// Is called when the window resizes
static void GLFW_Callback_WindowResize(GLFWwindow * window, int width, int height)
{
	CFG::setPreference(0, width);
	CFG::setPreference(1, height);
}

dt_Engine::~dt_Engine()
{
	Shutdown();
}

dt_Engine::dt_Engine()
{
	m_Initialized = false;	
	m_lastTime = 0;
}

bool Initialize_Sharing()
{
	if (!m_Initialized_Sharing) {

		glfwSetErrorCallback(GLFW_Callback_Error);
		if (!glfwInit()) {
			glfwTerminate();
			MSG::Error(OTHER_ERROR, "Unable to initialize!");
			return false;
		}

		const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		// Create an invisible window for asset sharing
		glfwWindowHint(GLFW_RED_BITS, mainMode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mainMode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mainMode->blueBits);
		glfwWindowHint(GLFW_ALPHA_BITS, 0);
		glfwWindowHint(GLFW_REFRESH_RATE, mainMode->refreshRate);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, DT_DESIRED_OGL_VER_MAJOR);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, DT_DESIRED_OGL_VER_MINOR);
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
		glewInit();

		MSG::Statement(	"Engine Version: " +
						DT_ENGINE_VER_MAJOR + "." +
						DT_ENGINE_VER_MINOR + "." +
						DT_ENGINE_VER_PATCH	);
		MSG::Statement("Using OpenGL Version: " + string(reinterpret_cast<char const *>(glGetString(GL_VERSION))));
		MSG::Statement("Using GLSL Version: " + string(reinterpret_cast<char const *>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
		MSG::Statement("GL implementation provided by: " + string(reinterpret_cast<char const *>(glGetString(GL_VENDOR))));
		MSG::Statement("Using GPU: " + string(reinterpret_cast<char const *>(glGetString(GL_RENDERER))));

		m_Initialized_Sharing = true;
	}
	return m_Initialized_Sharing;
}

bool dt_Engine::Initialize()
{
	unique_lock<shared_mutex> write_lock(m_package.m_EngineMutex);
	if ((!m_Initialized) && Initialize_Sharing()) {	
		CFG::loadConfiguration();		
		m_package.m_Camera = new Camera();
		m_UpdaterThread = new thread(&dt_Engine::Updater_Thread, this);
		m_UpdaterThread->detach();
		
		const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		m_package.window_width = CFG::getPreference(CFG_ENUM::C_WINDOW_WIDTH);
		m_package.window_height = CFG::getPreference(CFG_ENUM::C_WINDOW_HEIGHT);
		const int maxWidth = mainMode->width, maxHeight = mainMode->height;
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		m_package.m_Context_Rendering = glfwCreateWindow(m_package.window_width, m_package.window_height, "Delta", NULL, m_Context_Sharing);
		glfwSetWindowPos(m_package.m_Context_Rendering, (maxWidth - m_package.window_width) / 2, (maxHeight - m_package.window_height) / 2);
		glfwMakeContextCurrent(m_package.m_Context_Rendering);
		glfwSetInputMode(m_package.m_Context_Rendering, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
		glfwSetCursorPos(m_package.m_Context_Rendering, 0, 0);
		
		// Create Systems
		System_Input *sysInput = new System_Input(&m_package);
		glfwSetWindowUserPointer(m_package.m_Context_Rendering, sysInput);
		glfwSetWindowSizeCallback(m_package.m_Context_Rendering, GLFW_Callback_WindowResize);
		glfwSetCursorPosCallback(m_package.m_Context_Rendering, [](GLFWwindow * window, double x, double y) {
			static_cast<System_Input*>(glfwGetWindowUserPointer(window))->Callback_CursorPos(window, x, y);
		});
		glfwSetKeyCallback(m_package.m_Context_Rendering, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			static_cast<System_Input*>(glfwGetWindowUserPointer(window))->Callback_KeyPress(window, key, scancode, action, mods);
		});
		glfwSetCharModsCallback(m_package.m_Context_Rendering, [](GLFWwindow* window, unsigned int codepoint, int mods) {
			static_cast<System_Input*>(glfwGetWindowUserPointer(window))->Callback_CharMods(window, codepoint, mods);
		});
		glfwSetMouseButtonCallback(m_package.m_Context_Rendering, [](GLFWwindow * window, int button, int action, int mods) {
			static_cast<System_Input*>(glfwGetWindowUserPointer(window))->Callback_MouseButton(window, button, action, mods);
		});
		glfwSetScrollCallback(m_package.m_Context_Rendering, [](GLFWwindow * window, double xoffset, double yoffset) {
			static_cast<System_Input*>(glfwGetWindowUserPointer(window))->Callback_Scroll(window, xoffset, yoffset);
		});
		
		m_package.m_Systems.insert(pair<const char*, System*>("Input", sysInput));
		m_package.m_Systems.insert(pair<const char*, System*>("Graphics", new System_Graphics_PBR(&m_package)));
		m_package.m_Systems.insert(pair<const char*, System*>("Visibility", new System_Visibility(&m_package)));
		m_package.m_Systems.insert(pair<const char*, System*>("Logic", new System_Logic(&m_package)));

		Material_Manager::startup();
		Shadowmap_Manager::startup();
		World_Manager::startup();
		m_Initialized = true;
	}
	return m_Initialized;
}

void dt_Engine::Shutdown()
{
	unique_lock<shared_mutex> write_lock(m_package.m_EngineMutex);
	if (m_Initialized) {
		m_Initialized = false;

		if (m_UpdaterThread->joinable())
			m_UpdaterThread->join();
		delete m_UpdaterThread;

		write_lock.unlock();
		write_lock.release();
	}
}

void dt_Engine::Update()
{
	float deltaTime = 0;
	float thisTime = glfwGetTime();
	if (m_Initialized && !glfwWindowShouldClose(m_package.m_Context_Rendering)) {
		deltaTime = thisTime - m_lastTime;
		m_lastTime = thisTime;

		Asset_Manager::ParseWorkOrders();
		for each (auto system in m_package.m_Systems)
			system.second->Update(deltaTime);
		
		glfwSwapBuffers(m_package.m_Context_Rendering);
		glfwPollEvents();
	}
}

void dt_Engine::Updater_Thread()
{
	float lastTime = 0, thisTime = 0, deltaTime = 0;
	bool stay_alive = true;
	while (stay_alive) {
		shared_lock<shared_mutex> read_lock(m_package.m_EngineMutex);
		if (m_Initialized) {
			thisTime = glfwGetTime();
			deltaTime = thisTime - lastTime;
			lastTime = thisTime;
			for each (auto system in m_package.m_Systems)
				system.second->Update_Threaded(deltaTime);
		}
		stay_alive = !ShouldClose();
	}
}


bool dt_Engine::ShouldClose()
{
	return glfwWindowShouldClose(m_package.m_Context_Rendering);
}
