#include "Engine.h"
#include "Systems\World\Camera.h"
#include "Systems\System_Interface.h"
#include "Utilities\EnginePackage.h"
#include "Managers\Asset_Manager.h"
#include "Managers\Material_Manager.h"
#include "Managers\Message_Manager.h"
#include <string>

// OpenGL Dependent Systems //
#include "GL\glew.h"
#include "GLFW\glfw3.h"

static bool				m_Initialized_Sharing = false;
static GLFWwindow	*	m_Context_Sharing = nullptr;


static void GLFW_Callback_Error(int error, const char * description)
{
	MSG_Manager::Error(MSG_Manager::GLFW_ERROR, "(" + to_string(error) + "): " + description);
}

// Is called when the window resizes
static void GLFW_Callback_Windowresize(GLFWwindow * window, int width, int height)
{
	EnginePackage &package = *((EnginePackage*)glfwGetWindowUserPointer(window));
	package.m_Camera.setDimensions(vec2(width, height));
	package.m_Camera.update();
	package.setPreference(PreferenceState::C_WINDOW_WIDTH, width);
	package.setPreference(PreferenceState::C_WINDOW_HEIGHT, height);
}

// Is called when error messages occur within OpenGL driver
static void APIENTRY OpenGL_DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	string errorType;
	string errorSeverity;
	string errorMessage = string(message, length);
	switch (type) {
		case GL_DEBUG_TYPE_ERROR:
			errorType = "ERROR";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			errorType = "DEPRECATED_BEHAVIOR";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			errorType = "UNDEFINED_BEHAVIOR";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			errorType = "PORTABILITY";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			errorType = "PERFORMANCE";
			break;
		case GL_DEBUG_TYPE_OTHER:
			errorType = "OTHER";
			break;
	}
	switch (severity) {
		case GL_DEBUG_SEVERITY_LOW:
			errorSeverity = "LOW";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			errorSeverity = "MEDIUM";
			break;
		case GL_DEBUG_SEVERITY_HIGH:
			errorSeverity = "HIGH";
			break;
	}
	//if (type == 1280) {
		MSG_Manager::Statement(errorMessage +"\nType: " + errorType + ", Severity: " + errorSeverity + ", id: " + std::to_string(id));
	//}
	//MSG_Manager::Error(OPENGL_ERROR, errorMessage, +"\nType: " + errorType + ", Severity: " + errorSeverity + ", id: " + std::to_string(id));
}

Engine::~Engine()
{
	if (m_Initialized) {
		m_package->removePrefCallback(PreferenceState::C_SHADOW_QUALITY, this);
	}
}

Engine::Engine()
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
			MSG_Manager::Error(MSG_Manager::OTHER_ERROR, "Unable to initialize!");
			return false;
		}

		const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		// Create an invisible window for asset sharing
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
		glewInit();

		MSG_Manager::Statement(	
			"Engine Version: " + string(ENGINE_VERSION)
		);
		MSG_Manager::Statement("Using OpenGL Version: " + string(reinterpret_cast<char const *>(glGetString(GL_VERSION))));
		MSG_Manager::Statement("Using GLSL Version: " + string(reinterpret_cast<char const *>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
		MSG_Manager::Statement("GL implementation provided by: " + string(reinterpret_cast<char const *>(glGetString(GL_VENDOR))));
		MSG_Manager::Statement("Using GPU: " + string(reinterpret_cast<char const *>(glGetString(GL_RENDERER))));

		/*glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGL_DebugMessageCallback, nullptr);
		GLuint unusedIds = 0;
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);*/

		m_Initialized_Sharing = true;
	}
	return m_Initialized_Sharing;
}

void Shutdown_Sharing()
{
	Material_Manager::Shut_Down();
	Asset_Manager::Shut_Down();
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
	if ((!m_Initialized) && Initialize_Sharing()) {
		m_package = new EnginePackage();
		m_package->m_Camera.setHorizontalFOV(110.0f);
		unique_lock<shared_mutex> write_lock(m_package->m_EngineMutex);	
		const float farPlane = m_package->addPrefCallback(PreferenceState::C_SHADOW_QUALITY, this, [&](const float &f) { m_package->m_Camera.setFarPlane(f); m_package->m_Camera.update(); });
		//m_package->m_Camera.setNearPlane(1.0f);
		m_package->m_Camera.setFarPlane(farPlane);
		m_package->m_Camera.update();
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
		m_package->m_Context_Rendering = glfwCreateWindow(1, 1, "Delta", NULL, m_Context_Sharing); 
		glfwSetWindowIcon(m_package->m_Context_Rendering, 0, NULL);
		glfwMakeContextCurrent(m_package->m_Context_Rendering);	
				
		// Create all the required systems
		auto &systems = m_package->m_Systems;
		systems["Preferences"] = new System_Preferences("preferences");
		systems["Graphics"] = new System_Graphics();
		systems["PerfCounter"] = new System_PerfCounter();
		systems["Input"] = new System_Input();
		systems["Logic"] = new System_Logic();
		systems["World"] = new System_World();

		// Initialize all systems
		for each (auto &system in systems)
			system.second->initialize(m_package);

		const float window_width = m_package->getPreference(PreferenceState::C_WINDOW_WIDTH);
		const float window_height = m_package->getPreference(PreferenceState::C_WINDOW_HEIGHT);
		const int maxWidth = mainMode->width, maxHeight = mainMode->height;
		glfwSetWindowSize(m_package->m_Context_Rendering, window_width, window_height);
		glfwSetWindowPos(m_package->m_Context_Rendering, (maxWidth - window_width) / 2, (maxHeight - window_height) / 2);
		glfwSetInputMode(m_package->m_Context_Rendering, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
		glfwSetCursorPos(m_package->m_Context_Rendering, 0, 0);
				
		glfwSetWindowUserPointer(m_package->m_Context_Rendering, m_package);
		glfwSetWindowSizeCallback(m_package->m_Context_Rendering, GLFW_Callback_Windowresize);		
		glfwSwapInterval(0);

		Material_Manager::Start_Up();
		Asset_Manager::Start_Up();

		m_Initialized = true;
		m_frameCount = 0;
		m_frameAccumulator = 0;
	}
	return m_Initialized;
}

void Engine::shutdown()
{
	unique_lock<shared_mutex> write_lock(m_package->m_EngineMutex);
	if (m_Initialized) {
		m_package->removePrefCallback(PreferenceState::C_DRAW_DISTANCE, this);
		
		for each (auto system in m_package->m_Systems)
			delete system.second;
		m_package->m_Systems.clear();

		m_Initialized = false;
		write_lock.unlock();
		write_lock.release();
	}
}

void Engine::tick()
{
 	float deltaTime = 0;
	float thisTime = glfwGetTime();
	if (m_Initialized && !glfwWindowShouldClose(m_package->m_Context_Rendering)) {
		deltaTime = thisTime - m_lastTime;
		m_lastTime = thisTime;

		// performance heuristics
		m_frameCount++;
		if (m_frameCount >= 100) {
			m_frameAccumulator /= 100.0f;
			MSG_Manager::Statement("Avg Frametime = " + to_string(m_frameAccumulator*1000.0f) + " ms");
			m_frameAccumulator = deltaTime;
			m_frameCount = 0;
		}
		else 
			m_frameAccumulator += deltaTime;		
		// end performance heuristics

		Asset_Manager::Notify_Observers();
		Material_Manager::Parse_Work_Orders();
		Asset_Manager::Get_Model_Manager()->update();
		for each (auto system in m_package->m_Systems) 			
			system.second->update(deltaTime);		
		
		glfwSwapBuffers(m_package->m_Context_Rendering);
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
			Asset_Manager::Finalize_Orders();
			shared_lock<shared_mutex> read_lock(m_package->m_EngineMutex);
			for each (auto system in m_package->m_Systems)
				system.second->updateThreaded(deltaTime);
		}
		stay_alive = !shouldClose();
	}
}

bool Engine::shouldClose()
{
	return glfwWindowShouldClose(m_package->m_Context_Rendering);
}

Camera * Engine::getCamera() 
{ 
	return &m_package->m_Camera; 
}
