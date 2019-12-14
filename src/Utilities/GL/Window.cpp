#include "Utilities/GL/Window.h"
#include "Engine.h"
#include <glad/glad.h>
#include "GLFW/glfw3.h"


constexpr int DESIRED_OGL_VER_MAJOR = 4;
constexpr int DESIRED_OGL_VER_MINOR = 5;

Window::~Window() noexcept
{
	*m_aliveIndicator = false;
	for (auto& [thread, promise, context] : m_threads) {
		promise.set_value();
		if (thread.joinable())
			thread.join();
		glfwDestroyWindow(context);
	}
	m_threads.clear();
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

Window::Window(Engine& engine) noexcept :
	m_engine(engine)
{
	// Initialize GLFW
	if (!glfwInit()) {
		engine.getManager_Messages().error("GLFW unable to initialize, shutting down...");
		glfwTerminate();
		exit(-1);
	}

	// Create main window
	const auto& mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
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
	m_window = glfwCreateWindow(1, 1, "", nullptr, nullptr);
	glfwMakeContextCurrent(m_window);
	glfwSetWindowIcon(m_window, 0, nullptr);
	glfwSetCursorPos(m_window, 0, 0);

	// Initialize GLAD
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		engine.getManager_Messages().error("GLAD unable to initialize, shutting down...");
		glfwTerminate();
		exit(-1);
	}

	// Preference Values
	m_refreshRate = float(glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);
	m_windowSize.x = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
	m_windowSize.y = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;
	auto& preferenceState = engine.getPreferenceState();
	preferenceState.getOrSetValue(PreferenceState::Preference::C_WINDOW_WIDTH, m_windowSize.x);
	preferenceState.getOrSetValue(PreferenceState::Preference::C_WINDOW_HEIGHT, m_windowSize.y);
	preferenceState.getOrSetValue(PreferenceState::Preference::C_WINDOW_REFRESH_RATE, m_refreshRate);
	preferenceState.getOrSetValue(PreferenceState::Preference::C_WINDOW_FULLSCREEN, m_useFullscreen);
	preferenceState.getOrSetValue(PreferenceState::Preference::C_VSYNC, m_vsync);

	// Preference Callbacks
	preferenceState.addCallback(PreferenceState::Preference::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float& f) noexcept {
		m_windowSize.x = int(f);
		configureWindow();
		});
	preferenceState.addCallback(PreferenceState::Preference::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float& f) noexcept {
		m_windowSize.y = int(f);
		configureWindow();
		});
	preferenceState.addCallback(PreferenceState::Preference::C_WINDOW_REFRESH_RATE, m_aliveIndicator, [&](const float& f) noexcept {
		m_refreshRate = f;
		configureWindow();
		});
	preferenceState.addCallback(PreferenceState::Preference::C_WINDOW_FULLSCREEN, m_aliveIndicator, [&](const float& f) noexcept {
		m_useFullscreen = f;
		configureWindow();
		});
	preferenceState.addCallback(PreferenceState::Preference::C_VSYNC, m_aliveIndicator, [&](const float& f) noexcept {
		m_vsync = f;
		glfwSwapInterval((int)f);
		});
	configureWindow();
	glfwSwapInterval((int)m_vsync);
	glfwSetInputMode(m_window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
	glfwSetWindowUserPointer(m_window, &m_engine);
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) noexcept {
		auto& preferences = static_cast<Engine*>(glfwGetWindowUserPointer(window))->getPreferenceState();
		preferences.setValue(PreferenceState::Preference::C_WINDOW_WIDTH, width);
		preferences.setValue(PreferenceState::Preference::C_WINDOW_HEIGHT, height);
		});
	glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) noexcept {
		static_cast<Engine*>(glfwGetWindowUserPointer(window))->getModule_UI().applyCursorPos(xPos, yPos);
		});
	glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) noexcept {
		static_cast<Engine*>(glfwGetWindowUserPointer(window))->getModule_UI().applyCursorButton(button, action, mods);
		});
	glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int character) noexcept {
		static_cast<Engine*>(glfwGetWindowUserPointer(window))->getModule_UI().applyChar(character);
		});
	glfwSetKeyCallback(m_window, [](GLFWwindow* window, int a, int b, int c, int d)noexcept {
		static_cast<Engine*>(glfwGetWindowUserPointer(window))->getModule_UI().applyKey(a, b, c, d);
		});
#ifdef DEBUG
	if (GLAD_GL_KHR_debug) {
		GLint v;
		glGetIntegerv(GL_CONTEXT_FLAGS, &v);
		if (v && GL_CONTEXT_FLAG_DEBUG_BIT) {
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			engine.getManager_Messages().statement(">>> KHR DEBUG MODE ENABLED <<<");
			auto myCallback = [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data) {
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
					(reinterpret_cast<MessageManager*>(const_cast<void*>(data)))->error(
						std::to_string(id) + ": " + std::string(_type) + " of " + std::string(_severity) + " severity, raised from " + std::string(_source) + ": " + std::string(msg, length));
			};
			glDebugMessageCallbackKHR(myCallback, &engine.getManager_Messages());
		}
	}
#endif

	initThreads();
}

void Window::initThreads() noexcept
{
	const unsigned int maxThreads = std::max(1u, std::thread::hardware_concurrency());
	for (unsigned int x = 0; x < maxThreads; ++x) {
		std::promise<void> exitSignal;
		std::future<void> exitObject = exitSignal.get_future();
		const auto& mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
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
		auto sharedContext = glfwCreateWindow(1, 1, "", nullptr, m_window);
		std::thread workerThread(&Engine::tickThreaded, &m_engine, std::move(exitObject), sharedContext);
		workerThread.detach();
		m_threads.push_back(std::move(std::tuple(std::move(workerThread), std::move(exitSignal), sharedContext)));
	}
}

bool Window::shouldClose() const noexcept
{
	return glfwWindowShouldClose(m_window);
}

void Window::close() noexcept
{
	glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

void Window::swapBuffers() noexcept
{
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}

glm::vec2 Window::getMousePos() const noexcept
{
	double mouseX, mouseY;
	glfwGetCursorPos(m_window, &mouseX, &mouseY);
	return glm::vec2(mouseX, mouseY);
}

void Window::setMousePos(const glm::vec2& position) noexcept
{
	glfwSetCursorPos(m_window, position.x, position.y);
}

bool Window::getMouseKey(const int& buttonID) const noexcept
{
	return (bool)(glfwGetMouseButton(m_window, buttonID));
}

bool Window::getKey(const int& buttonID) const noexcept
{
	return (bool)(glfwGetKey(m_window, buttonID));
}

void Window::setMouseMode3D(const bool& use3DMouse) noexcept
{
	glfwSetInputMode(m_window, GLFW_CURSOR, use3DMouse ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

GLFWwindow* Window::getContext() const noexcept
{
	return m_window;
}

void Window::MakeCurrent(GLFWwindow* context) noexcept
{
	glfwMakeContextCurrent(context);
}

float Window::GetSystemTime() noexcept
{
	return (float)glfwGetTime();
}

std::string Window::GetVersion() noexcept
{
	return std::string(glfwGetVersionString(), 5);
}

std::vector<glm::ivec3> Window::GetResolutions() noexcept
{
	int count(0);
	const GLFWvidmode* modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
	std::vector<glm::ivec3> resolutions(count);
	for (int x = 0; x < count; ++x)
		resolutions[x] = { modes[x].width, modes[x].height, modes[x].refreshRate };
	return resolutions;
}

void Window::configureWindow() noexcept
{
	const GLFWvidmode* mainMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	const int maxWidth = mainMode->width, maxHeight = mainMode->height;
	glfwSetWindowSize(m_window, m_windowSize.x, m_windowSize.y);
	glfwSetWindowPos(m_window, (maxWidth - m_windowSize.x) / 2, (maxHeight - m_windowSize.y) / 2);
	glfwSetWindowMonitor(
		m_window,
		m_useFullscreen ? glfwGetPrimaryMonitor() : nullptr,
		0, 0,
		m_windowSize.x, m_windowSize.y,
		(int)m_refreshRate
	);
}