#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include "glm/glm.hpp"
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <vector>


// Forward Declarations
struct GLFWwindow;
class Engine;

/** Encapsulates an operating system's application window. */
class Window {
public:
	// Public (De)Constructors
	/** Destroy this window. */
	~Window();
	/** Construct a window. */
	explicit Window(Engine& engine);


	// Public Methods
	/** Checks if this window wants to shut down.
	@return				true if the window should close. */
	bool shouldClose() const noexcept;
	/** Close this window. */
	void close() noexcept;
	/** Finish processing a new frame. */
	void swapBuffers() noexcept;
	/** Retrieve the XY coordinates of the mouse relative to this window. 
	@return				the coordinates of the mouse. */
	glm::vec2 getMousePos() const noexcept;
	/** Set the XY coordinates of the mouse relative to this window. 
	@param	position	the coordinates of the mouse. */
	void setMousePos(const glm::vec2& position) noexcept;
	/** Retrieve the button-press state of a mouse button.
	@param	buttonID	the enumerated button of the mouse, starting at 0 for left click.
	@return				true if pressed, false otherwise. */
	bool getMouseKey(const int& buttonID) const noexcept;
	/** Retrieve the button-press state of a keyboard button.
	@param	buttonID	the enumerated button of the keyboard.
	@return				true if pressed, false otherwise. */
	bool getKey(const int& buttonID) const noexcept;
	/** Set whether or not the mouse should be clamped to the center of the window for 3D movement. 
	@param	use3DMouse	set to true to clamp the mouse to the center. */
	void setMouseMode3D(const bool& use3DMouse) noexcept;
	/** Retrieve this engine's rendering context.
	@return				this engine's rendering context. */
	GLFWwindow* getContext() const noexcept;
	/** Make the supplied context current for OpenGL functions. 
	@param	context		pointer to the desired context. */
	static void MakeCurrent(GLFWwindow* const context) noexcept;
	/** Retrieve the current time.
	@return				the current time. */
	static float GetSystemTime() noexcept;
	/** Retrieve the GLFW version.
	@return				the version of GLFW in use. */
	static std::string GetVersion();
	/** Retrieve a list of available resolutions.
	@return				vector of supported resolutions. */
	static std::vector<glm::ivec3> GetResolutions();


private:
	// Private but deleted
	/** Disallow window move constructor. */
	inline Window(Window&&) noexcept = delete;
	/** Disallow window copy constructor. */
	inline Window(const Window&) noexcept = delete;
	/** Disallow window move assignment. */
	inline Window& operator =(Window&&) noexcept = delete;
	/** Disallow window copy assignment. */
	inline Window& operator =(const Window&) noexcept = delete;


	// Private Methods
	/** Initialize the auxiliary processing threads. */
	void initThreads();
	/** Updates the window attributes. */
	void configureWindow() noexcept;


	// Private Attributes
	Engine& m_engine;
	GLFWwindow* m_window = nullptr;
	float m_refreshRate = 120.0f;
	float m_useFullscreen = 1.0f;
	float m_vsync = 1.0f;
	glm::ivec2 m_windowSize = glm::ivec2(1);
	std::vector<std::tuple<std::thread, std::promise<void>, GLFWwindow*>> m_threads;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // WINDOW_H