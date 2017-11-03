#include "dt_Core.h"
#include "Managers\Asset_Manager.h"
#include "Managers\Config_Manager.h"
#include "Managers\Message_Manager.h"

// OpenGL Dependent Systems //
#include "GL\glew.h"
#include "GLFW\glfw3.h"
#include "Managers\Material_Manager.h"

#include <string>

static GLFWwindow *asset_sharing_context = nullptr;

// GLFW sends its errors here
static void error_callback(int error, const char* description)
{
	MSG::Error(GLFW_ERROR, "(" + to_string(error) + "): " + description);	
}

namespace dt_Core {

	bool Initialize() 
	{
		//Shared_Asset_Primitive qwe;
		//Asset_Manager::load_asset(qwe, "quad", false);
		glfwSetErrorCallback(error_callback);
		if (!glfwInit()) {
			glfwTerminate();
			return false;
		}

		// Create an invisible window for asset sharing
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, DT_DESIRED_OGL_VER_MAJOR);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, DT_DESIRED_OGL_VER_MINOR);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

		asset_sharing_context = glfwCreateWindow(640, 480, "", NULL, NULL);
		glfwMakeContextCurrent(asset_sharing_context);
		glewExperimental = GL_TRUE;
		glewInit();

		// OpenGL Dependent Systems //
		CFG::loadConfiguration();
		Material_Manager::startup();
		
		MSG::Statement(	"Engine Version: " 
						+ DT_ENGINE_VER_MAJOR  + "." 
						+ DT_ENGINE_VER_MINOR  + "." 
						+ DT_ENGINE_VER_PATCH);
		MSG::Statement("Using OpenGL Version: " + string(reinterpret_cast<char const *>(glGetString(GL_VERSION))));
		MSG::Statement("Using GLSL Version: " + string(reinterpret_cast<char const *>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
		MSG::Statement("GL implementation provided by: " + string(reinterpret_cast<char const *>(glGetString(GL_VENDOR))));
		MSG::Statement("Using GPU: " + string(reinterpret_cast<char const *>(glGetString(GL_RENDERER))));

		return true;
	}
	void Shutdown()
	{
		Asset_Manager::shutdown();
		CFG::shutdown();
		glfwMakeContextCurrent(asset_sharing_context);
		glfwTerminate();
	}
	void* GetContext()
	{
		return asset_sharing_context;
	}
}