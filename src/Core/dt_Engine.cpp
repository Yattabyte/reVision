#include "dt_Engine.h"
#include "Rendering\Camera.h"
#include "Systems\System_Interface.h"
#include <string>

#include "Utilities\Engine_Package.h"

#include "Systems\Input\Input.h"
#include "Systems\Graphics\Graphics_PBR.h"
#include "Systems\Visibility.h"
#include "Systems\Logic\Logic.h"
#include "Systems\Preferences\Preferences.h"
#include "Systems\World\World.h"


// To replace with abstract systems
#include "Systems\Message_Manager.h"
#include "Systems\Asset_Manager.h"
#include "Systems\Material_Manager.h"
#include "Systems\Shadowmap_Manager.h"

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
	Engine_Package &package = *((Engine_Package*)glfwGetWindowUserPointer(window));
	package.SetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH, width);
	package.SetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT, height);
	package.m_Camera.setDimensions(vec2(width, height));
	package.m_Camera.Update();
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
	if ((!m_Initialized) && Initialize_Sharing()) {

		m_package = new Engine_Package();
		unique_lock<shared_mutex> write_lock(m_package->m_EngineMutex);		
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		m_package->m_Context_Rendering = glfwCreateWindow(1, 1, "Delta", NULL, m_Context_Sharing);
		glfwMakeContextCurrent(m_package->m_Context_Rendering);

		m_package->m_Systems.insert(pair<const char*, System*>("Preferences", new System_Preferences(m_package, "preferences")));
		m_package->m_Systems.insert(pair<const char*, System*>("Input", new System_Input(m_package)));
		m_package->m_Systems.insert(pair<const char*, System*>("Visibility", new System_Visibility(m_package)));
		m_package->m_Systems.insert(pair<const char*, System*>("Logic", new System_Logic(m_package)));
		m_package->m_Systems.insert(pair<const char*, System*>("Graphics", new System_Graphics_PBR(m_package)));
		m_package->m_Systems.insert(pair<const char*, System*>("World", new System_World(m_package)));

		const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		const float window_width = m_package->GetPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH);
		const float window_height = m_package->GetPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT);
		const int maxWidth = mainMode->width, maxHeight = mainMode->height;
		glfwSetWindowSize(m_package->m_Context_Rendering, window_width, window_height);
		glfwSetWindowPos(m_package->m_Context_Rendering, (maxWidth - window_width) / 2, (maxHeight - window_height) / 2);
		glfwSetInputMode(m_package->m_Context_Rendering, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
		glfwSetCursorPos(m_package->m_Context_Rendering, 0, 0);
				
		glfwSetWindowUserPointer(m_package->m_Context_Rendering, m_package);
		glfwSetWindowSizeCallback(m_package->m_Context_Rendering, GLFW_Callback_WindowResize);		

		m_UpdaterThread = new thread(&dt_Engine::Updater_Thread, this);
		m_UpdaterThread->detach();

		Material_Manager::startup();
		Shadowmap_Manager::startup();
		m_Initialized = true;
	}
	return m_Initialized;
}

void dt_Engine::Shutdown()
{
	unique_lock<shared_mutex> write_lock(m_package->m_EngineMutex);
	if (m_Initialized) {
		m_Initialized = false;

		if (m_UpdaterThread->joinable())
			m_UpdaterThread->join();
		delete m_UpdaterThread;
		
		for each (auto system in m_package->m_Systems)
			system.second->Shutdown();

		write_lock.unlock();
		write_lock.release();
	}
}

void dt_Engine::Update()
{
	float deltaTime = 0;
	float thisTime = glfwGetTime();
	if (m_Initialized && !glfwWindowShouldClose(m_package->m_Context_Rendering)) {
		deltaTime = thisTime - m_lastTime;
		m_lastTime = thisTime;

		Asset_Manager::ParseWorkOrders();
		for each (auto system in m_package->m_Systems)
			system.second->Update(deltaTime);
		
		glfwSwapBuffers(m_package->m_Context_Rendering);
		glfwPollEvents();
	}
}

void dt_Engine::Updater_Thread()
{
	float lastTime = 0, thisTime = 0, deltaTime = 0;
	bool stay_alive = true;
	while (stay_alive) {
		shared_lock<shared_mutex> read_lock(m_package->m_EngineMutex);
		if (m_Initialized) {
			thisTime = glfwGetTime();
			deltaTime = thisTime - lastTime;
			lastTime = thisTime;
			for each (auto system in m_package->m_Systems)
				system.second->Update_Threaded(deltaTime);
		}
		stay_alive = !ShouldClose();
	}
}


bool dt_Engine::ShouldClose()
{
	return glfwWindowShouldClose(m_package->m_Context_Rendering);
}

Camera * dt_Engine::GetCamera() 
{ 
	return &m_package->m_Camera; 
}
