#include "Systems\Input\Input.h"
#include "GLFW\glfw3.h"
#include "Input.h"



System_Input::~System_Input()
{
}

System_Input::System_Input(GLFWwindow * window, Action_State * state, const System_Input_Binding & binds)
	:	m_window(window),
		m_state(state),
		m_binds(binds)
{

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
		m_state->at(action) = (glfwGetKey(m_window, input_button)) ? 1.0f : 0.0f;
	}
}

void System_Input::Callback_CursorPos(GLFWwindow * window, double x, double y)
{
	bool qwe = true;
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