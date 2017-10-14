#include "Rendering\Viewport.h"
#include "Managers\Config_Manager.h"
#include "Managers\Message_Manager.h"
#include <stdlib.h>
#include <iostream>

// OpenGL Dependent Systems //
#include "Managers\Material_Manager.h"

// GLFW sends its errors here
static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

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
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		close_callback(0); return;
	}
	const double	width = CFG::getPreference(CFG_ENUM::C_WINDOW_WIDTH),
		height = CFG::getPreference(CFG_ENUM::C_WINDOW_HEIGHT);

	GLFWmonitor *mainMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mainMode = glfwGetVideoMode(mainMonitor);
	glfwWindowHint(GLFW_RED_BITS, mainMode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mainMode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mainMode->blueBits);
	glfwWindowHint(GLFW_ALPHA_BITS, 0);
	glfwWindowHint(GLFW_REFRESH_RATE, mainMode->refreshRate);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	window = glfwCreateWindow(width, height, "Viewport", NULL, NULL);
	if (!window) {
		close_callback(0); return;
	}
	glfwSetWindowPos(window, ((mainMode->width - width) / 2), ((mainMode->height - height) / 2));
	glfwMakeContextCurrent(window);
	glfwSetWindowCloseCallback(window, close_callback);
	glewExperimental = GL_TRUE;
	glewInit();
	glfwSwapInterval(0);
	glfwMakeContextCurrent(window);

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	MSG::Statement(std::string(reinterpret_cast<char const *>(renderer)));
	MSG::Statement(std::string(reinterpret_cast<char const *>(version)));

	//setWindowSize(vec2(width, height));
	//ImGui_ImplGlfwGL3_Init(window, false);

	// OpenGL Dependent Systems //
	Material_Manager::startup();
}

bool Viewport::ShouldClose() const
{
	return glfwWindowShouldClose(window);
}

