#include "Rendering\Viewport.h"
#include "Managers\Config_Manager.h"
#include "Managers\Message_Manager.h"
#include "dt_Core.h"
#include "GLFW\glfw3.h"

// Is called when the @window closes
static void close_callback(GLFWwindow * window)
{
	glfwTerminate();
}

Viewport::~Viewport()
{
}

Viewport::Viewport()
{
	
}

void Viewport::Initialize()
{	
	const double	width	= CFG::getPreference(CFG_ENUM::C_WINDOW_WIDTH),
					height	= CFG::getPreference(CFG_ENUM::C_WINDOW_HEIGHT);

	GLFWmonitor *mainMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mainMode = glfwGetVideoMode(mainMonitor);
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
	GLFWwindow *other = (GLFWwindow*)dt_Core::GetContext();
	window = glfwCreateWindow(width, height, "Viewport", NULL, other);
	if (!window) {
		close_callback(0); 
		return;
	}
	glfwSetWindowPos(window, ((mainMode->width - width) / 2), ((mainMode->height - height) / 2));
	glfwMakeContextCurrent(window);
	glfwSetWindowCloseCallback(window, close_callback);
	glfwSwapInterval(0);
}

bool Viewport::ShouldClose() const
{
	return glfwWindowShouldClose(window);
}

