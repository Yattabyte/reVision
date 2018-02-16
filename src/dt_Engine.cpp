#include "dt_Engine.h"
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

static void GLFW_Callback_Error(int error, const char* description)
{
	MSG::Error(GLFW_ERROR, "(" + to_string(error) + "): " + description);
}

// Is called when the window resizes
static void GLFW_Callback_WindowResize(GLFWwindow * window, int width, int height)
{
	EnginePackage &package = *((EnginePackage*)glfwGetWindowUserPointer(window));
	package.setPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH, width);
	package.setPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT, height);
	package.m_Camera.setDimensions(vec2(width, height));
	package.m_Camera.update();
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
//	MSG::Error(OPENGL_ERROR, errorMessage, +"\nType: " + errorType + ", Severity: " + errorSeverity + ", id: " + std::to_string(id));
}

class EN_DrawDistCallback : public Callback_Container {
public:
	~EN_DrawDistCallback() {};
	EN_DrawDistCallback(EnginePackage *pointer) : m_pointer(pointer) {}
	void Callback(const float &value) {
		m_pointer->m_Camera.setFarPlane(value);
		m_pointer->m_Camera.update();
	}
private:
	EnginePackage *m_pointer;
};

dt_Engine::~dt_Engine()
{
	shutdown();
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

		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGL_DebugMessageCallback, nullptr);
		GLuint unusedIds = 0;
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, false);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, &unusedIds, true);

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
bool dt_Engine::initialize(const vector<pair<const char*, System*>> &systems)
{
	if ((!m_Initialized) && Initialize_Sharing()) {

		m_package = new EnginePackage();
		unique_lock<shared_mutex> write_lock(m_package->m_EngineMutex);		

		m_drawDistCallback = new EN_DrawDistCallback(m_package);
		m_package->addCallback(PREFERENCE_ENUMS::C_DRAW_DISTANCE, m_drawDistCallback);
		const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

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
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		m_package->m_Context_Rendering = glfwCreateWindow(1, 1, "Delta", NULL, m_Context_Sharing);
		glfwMakeContextCurrent(m_package->m_Context_Rendering);		
		
		for each (auto &pair in systems) {
			pair.second->Initialize(m_package);
			m_package->m_Systems.insert(std::pair<const char*, System*>(pair.first, pair.second));
		}

		const float window_width = m_package->getPreference(PREFERENCE_ENUMS::C_WINDOW_WIDTH);
		const float window_height = m_package->getPreference(PREFERENCE_ENUMS::C_WINDOW_HEIGHT);
		const int maxWidth = mainMode->width, maxHeight = mainMode->height;
		glfwSetWindowSize(m_package->m_Context_Rendering, window_width, window_height);
		glfwSetWindowPos(m_package->m_Context_Rendering, (maxWidth - window_width) / 2, (maxHeight - window_height) / 2);
		glfwSetInputMode(m_package->m_Context_Rendering, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
		glfwSetCursorPos(m_package->m_Context_Rendering, 0, 0);
				
		glfwSetWindowUserPointer(m_package->m_Context_Rendering, m_package);
		glfwSetWindowSizeCallback(m_package->m_Context_Rendering, GLFW_Callback_WindowResize);		

		Material_Manager::Start_Up();
		Asset_Manager::Start_Up();
		m_UpdaterThread = new thread(&dt_Engine::Updater_Thread, this);
		m_UpdaterThread->detach();

		m_Initialized = true;
	}
	return m_Initialized;
}

void dt_Engine::shutdown()
{
	unique_lock<shared_mutex> write_lock(m_package->m_EngineMutex);
	if (m_Initialized) {
		m_package->removeCallback(PREFERENCE_ENUMS::C_DRAW_DISTANCE, m_drawDistCallback);
		delete m_drawDistCallback;
		
		if (m_UpdaterThread->joinable())
			m_UpdaterThread->join();
		delete m_UpdaterThread;
		
		for each (auto system in m_package->m_Systems)
			delete system.second;
		m_package->m_Systems.clear();

		m_Initialized = false;
		write_lock.unlock();
		write_lock.release();
	}
}

void dt_Engine::update()
{
 	float deltaTime = 0;
	float thisTime = glfwGetTime();
	if (m_Initialized && !glfwWindowShouldClose(m_package->m_Context_Rendering)) {
		deltaTime = thisTime - m_lastTime;
		m_lastTime = thisTime;

		glfwMakeContextCurrent(m_package->m_Context_Rendering);
		Asset_Manager::Notify_Observers();
		Material_Manager::Parse_Work_Orders();
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
		if (m_Initialized && m_Initialized_Sharing) {
			thisTime = glfwGetTime();
			deltaTime = thisTime - lastTime;
			lastTime = thisTime;
			glfwMakeContextCurrent(m_Context_Sharing);
			Asset_Manager::Finalize_Orders();
			shared_lock<shared_mutex> read_lock(m_package->m_EngineMutex);
			for each (auto system in m_package->m_Systems)
				system.second->Update_Threaded(deltaTime);
			//glFinish();
		}
		stay_alive = !shouldClose();
	}
}

bool dt_Engine::shouldClose()
{
	return glfwWindowShouldClose(m_package->m_Context_Rendering);
}

Camera * dt_Engine::getCamera() 
{ 
	return &m_package->m_Camera; 
}
