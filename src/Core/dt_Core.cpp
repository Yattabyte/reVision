#include "dt_Core.h"
#include "Rendering\Camera.h"
#include "Rendering\Scenes\Scene.h"
#include "Managers\Asset_Manager.h"
#include "Managers\Config_Manager.h"
#include "Managers\Geometry_Manager.h"
#include "Managers\Lighting_Manager.h"
#include "Managers\Material_Manager.h"
#include "Managers\Message_Manager.h"
#include "Managers\Shadowmap_Manager.h"
#include "Managers\Visibility_Manager.h"
#include "Managers\World_Manager.h"

// OpenGL Dependent Systems //
#include "GL\glew.h"
#include "GLFW\glfw3.h"

#include <string>

static bool			should_close			= false;
static GLFWwindow	*rendering_context		= nullptr,
					*asset_sharing_context	= nullptr;
static Scene		*rendering_scene		= nullptr;
static Camera		*rendering_camera		= nullptr;

// GLFW sends its errors here
static void GLFW_error_callback(int error, const char* description)
{
	MSG::Error(GLFW_ERROR, "(" + to_string(error) + "): " + description);	
}

// Is called when the window closes
static void GLFW_window_close_callback(GLFWwindow * window)
{
	should_close = true;
}

// Is called when the window resizes
static void GLFW_window_resize_callback(GLFWwindow * window, int width, int height)
{
	CFG::setPreference(0, width);
	CFG::setPreference(1, height);
}

namespace dt_Core {

	bool Initialize() 
	{
		//Shared_Asset_Primitive qwe;
		//Asset_Manager::load_asset(qwe, "quad", false);
		glfwSetErrorCallback(GLFW_error_callback);
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
		asset_sharing_context = glfwCreateWindow(512, 512, "", NULL, NULL);
		glfwMakeContextCurrent(asset_sharing_context);
		glewExperimental = GL_TRUE;
		glewInit();

		CFG::loadConfiguration();
		
		MSG::Statement(	"Engine Version: " 
						+ DT_ENGINE_VER_MAJOR  + "." 
						+ DT_ENGINE_VER_MINOR  + "." 
						+ DT_ENGINE_VER_PATCH);
		MSG::Statement("Using OpenGL Version: " + string(reinterpret_cast<char const *>(glGetString(GL_VERSION))));
		MSG::Statement("Using GLSL Version: " + string(reinterpret_cast<char const *>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
		MSG::Statement("GL implementation provided by: " + string(reinterpret_cast<char const *>(glGetString(GL_VENDOR))));
		MSG::Statement("Using GPU: " + string(reinterpret_cast<char const *>(glGetString(GL_RENDERER))));
		
		const double	width	= CFG::getPreference(CFG_ENUM::C_WINDOW_WIDTH),
						height	= CFG::getPreference(CFG_ENUM::C_WINDOW_HEIGHT);
		int maxWidth = mainMode->width, maxHeight = mainMode->height;
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		rendering_context = glfwCreateWindow(width, height, "Delta", NULL, asset_sharing_context);
		glfwSetWindowPos(rendering_context, ((maxWidth - width) / 2), ((maxHeight - height) / 2));
		glfwMakeContextCurrent(rendering_context);
		glfwSetWindowCloseCallback(rendering_context, GLFW_window_close_callback);
		glfwSetWindowSizeCallback(rendering_context, GLFW_window_resize_callback);

		Material_Manager::startup();
		Shadowmap_Manager::startup();
		Visibility_Manager::statup();
		return true;
	}

	void Shutdown()
	{
		Visibility_Manager::shutdown();
		Asset_Manager::shutdown();		
		CFG::shutdown();
		World_Manager::shutdown();
		Geometry_Manager::shutdown();
		Lighting_Manager::shutdown();
		Shadowmap_Manager::shutdown();
		glfwMakeContextCurrent(asset_sharing_context);
		glfwTerminate();
	}

	void Tick()
	{
		if (!(glfwWindowShouldClose(rendering_context) && should_close)) {
			Asset_Manager::ParseWorkOrders();
			if (rendering_scene != nullptr) {
				rendering_scene->RenderFrame();
			}
			glfwSwapBuffers(rendering_context);
		}
	}

	bool ShouldClose() 
	{
		return should_close;
	}

	void SetScene(Scene * scene)
	{
		rendering_scene = scene;
	}

	void SetCamera(Camera * camera)
	{
		rendering_camera = camera;
	}

	Camera * GetCamera()
	{
		return rendering_camera;
	}
}