#include "Systems\Input\Input.h"
#include "Utilities\Engine_Package.h"
#include "GLFW\glfw3.h"
#include "Input.h"



System_Input::~System_Input()
{
}

System_Input::System_Input(const System_Input_Binding & binds) : m_binds(binds)
{
}

void System_Input::Initialize(Engine_Package * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage; 
		m_Initialized = true;
	}
}

void System_Input::Update(const float & deltaTime)
{
	const auto bindings = m_binds.getBindings();
	if (!bindings.get()) return;
	const auto &bind_map = bindings.get()->configuration;
	// Pair is the action followed by the key assigned to it
	for each (const auto &pair in bind_map) {		
		const auto &action = pair.first;
		const auto &input_button = (int)pair.second;
		// If Key is pressed, set state to 1, otherwise set to 0
		m_enginePackage->m_Action_State.at(action) = (glfwGetKey(m_enginePackage->m_Context_Rendering, input_button)) ? 1.0f : 0.0f;
	}
	double mouseX, mouseY;
	glfwGetCursorPos(m_enginePackage->m_Context_Rendering, &mouseX, &mouseY);
	m_enginePackage->m_Action_State.at(LOOK_X) = mouseX;
	m_enginePackage->m_Action_State.at(LOOK_Y) = mouseY;
	glfwSetCursorPos(m_enginePackage->m_Context_Rendering, 0, 0);
}

void System_Input::Callback_CursorPos(GLFWwindow * window, double x, double y)
{
	
}

void System_Input::Callback_KeyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	bool qwe = true;
}

void System_Input::Callback_CharMods(GLFWwindow* window, unsigned int codepoint, int mods)
{
	bool qwe = true;
}

void System_Input::Callback_MouseButton(GLFWwindow* window, int button, int action, int mods)
{
	bool qwe = true;
}

void System_Input::Callback_Scroll(GLFWwindow* window, double xoffset, double yoffset)
{
	bool qwe = true;
}