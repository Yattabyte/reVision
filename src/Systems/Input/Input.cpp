#include "Systems\Input\Input.h"
#include "Engine.h"
#include "GLFW\glfw3.h"
#include "Input.h"


System_Input::~System_Input()
{
}

System_Input::System_Input(Engine * engine, const std::string & filename) : m_binds(InputBinding(engine, filename))
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
	auto & actionState = m_engine->getActionState();
	auto * renderingContext = m_engine->getRenderingContext();
	const auto bindings = m_binds.getBindings();
	if (!bindings.get()) return;
	const auto &bind_map = bindings.get()->m_configuration;
	// Pair is the action followed by the key assigned to it
	for each (const auto &pair in bind_map) {		
		const auto &action = pair.first;
		const auto &input_button = (int)pair.second;
		// If Key is pressed, set state to 1, otherwise set to 0
		actionState.at(action) = (glfwGetKey(renderingContext, input_button)) ? 1.0f : 0.0f;
	}
	double mouseX, mouseY;
	glfwGetCursorPos(renderingContext, &mouseX, &mouseY);
	actionState.at(ActionState::LOOK_X) = mouseX;
	actionState.at(ActionState::LOOK_Y) = mouseY;
	glfwSetCursorPos(renderingContext, 0, 0);
}