#include "dt_Engine.h"
#include "Rendering\Camera.h"
#include "Rendering\Scenes\Scene.h"
#include "Systems\System_Interface.h"
#include <string>

#include "Systems\Message_Manager.h"
#include "Systems\Input\Input.h"
#include "Systems\Asset_Manager.h"


// To replace with abstract systems
#include "Systems\Config_Manager.h"
#include "Systems\Material_Manager.h"
#include "Systems\Shadowmap_Manager.h"
#include "Systems\Visibility_Manager.h"
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
	m_Context_Rendering = nullptr;
	m_Scene = nullptr;
	m_Camera = nullptr;
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
		CFG::loadConfiguration();		
		
		const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		const double	width		= CFG::getPreference(CFG_ENUM::C_WINDOW_WIDTH),
						height		= CFG::getPreference(CFG_ENUM::C_WINDOW_HEIGHT);
		const int		maxWidth	= mainMode->width, maxHeight = mainMode->height;
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		m_Context_Rendering = glfwCreateWindow(width, height, "Delta", NULL, m_Context_Sharing);
		glfwSetWindowPos(m_Context_Rendering, ((maxWidth - width) / 2), ((maxHeight - height) / 2));
		glfwMakeContextCurrent(m_Context_Rendering);
		glfwSetInputMode(m_Context_Rendering, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		
		// Create Systems
		System_Input *sysInput = new System_Input(m_Context_Rendering, &m_Action_State);
		m_Systems.push_back(sysInput);

		// Set Callbacks
		glfwSetWindowUserPointer(m_Context_Rendering, sysInput);
		glfwSetWindowSizeCallback(m_Context_Rendering, GLFW_Callback_WindowResize);		
		glfwSetCursorPosCallback(m_Context_Rendering, [](GLFWwindow * window, double x, double y) { 
			static_cast<System_Input*>(glfwGetWindowUserPointer(window))->Callback_CursorPos(window, x, y);
		});
		glfwSetKeyCallback(m_Context_Rendering, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			static_cast<System_Input*>(glfwGetWindowUserPointer(window))->Callback_KeyPress(window, key, scancode, action, mods);
		});
		glfwSetCharModsCallback(m_Context_Rendering, [](GLFWwindow* window, unsigned int codepoint, int mods) {
			static_cast<System_Input*>(glfwGetWindowUserPointer(window))->Callback_CharMods(window, codepoint, mods);
		});
		glfwSetMouseButtonCallback(m_Context_Rendering, [](GLFWwindow * window, int button, int action, int mods) {
			static_cast<System_Input*>(glfwGetWindowUserPointer(window))->Callback_MouseButton(window, button, action, mods);
		});
		glfwSetScrollCallback(m_Context_Rendering, [](GLFWwindow * window, double xoffset, double yoffset) {
			static_cast<System_Input*>(glfwGetWindowUserPointer(window))->Callback_Scroll(window, xoffset, yoffset);
		});

		Material_Manager::startup();
		Shadowmap_Manager::startup();
		Visibility_Manager::statup();
		World_Manager::startup();
		m_Initialized = true;
	}
	return m_Initialized;
}

void dt_Engine::Shutdown()
{
	if (m_Initialized) {
		m_Initialized = false;

		/* Shutdown Code */
	}
}

void dt_Engine::Tick(const float &deltaTime)
{
	if (!glfwWindowShouldClose(m_Context_Rendering)) {
		// Begin rendering frame first
		if (m_Scene != nullptr) {
			m_Scene->RenderFrame(m_Camera);
		}

		// Do other frames while gpu is busy (don't force it to sync yet)
		Asset_Manager::ParseWorkOrders();
		for each (auto system in m_Systems)
			system->Update(deltaTime);

		// Now sync
		glfwSwapBuffers(m_Context_Rendering);
		glfwPollEvents();
	}
}

bool dt_Engine::ShouldClose()
{
	return glfwWindowShouldClose(m_Context_Rendering);
}

void dt_Engine::SetScene(Scene * scene)
{
	m_Scene = scene;
}

void dt_Engine::SetCamera(Camera * camera)
{
	m_Camera = camera;
}
