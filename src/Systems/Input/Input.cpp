#include "Systems\Input\Input.h"
#include "Engine.h"
#include "GLFW\glfw3.h"
#include "Input.h"


System_Input::~System_Input()
{
}

System_Input::System_Input(const InputBinding & binds) : m_binds(binds)
{
}

void System_Input::initialize(Engine * engine)
{
	if (!m_Initialized) {
		m_engine = engine; 
		m_Initialized = true;
	}
}

void System_Input::update(const float & deltaTime)
{
	const auto bindings = m_binds.getBindings();
	if (!bindings.get()) return;
	const auto &bind_map = bindings.get()->m_configuration;
	// Pair is the action followed by the key assigned to it
	for each (const auto &pair in bind_map) {		
		const auto &action = pair.first;
		const auto &input_button = (int)pair.second;
		// If Key is pressed, set state to 1, otherwise set to 0
		m_engine->m_ActionState.at(action) = (glfwGetKey(m_engine->m_Context_Rendering, input_button)) ? 1.0f : 0.0f;
	}
	double mouseX, mouseY;
	glfwGetCursorPos(m_engine->m_Context_Rendering, &mouseX, &mouseY);
	m_engine->m_ActionState.at(ActionState::LOOK_X) = mouseX;
	m_engine->m_ActionState.at(ActionState::LOOK_Y) = mouseY;
	glfwSetCursorPos(m_engine->m_Context_Rendering, 0, 0);
}