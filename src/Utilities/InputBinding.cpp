#include "Utilities/InputBinding.h"
#include "Modules/UI/KeyboardEvent.h"
#include "Engine.h"


InputBinding::~InputBinding()
{
	save();
}

InputBinding::InputBinding(Engine * engine) : m_engine(engine) {}

void InputBinding::loadFile(const std::string & filename)
{
	m_config = Shared_Config(m_engine, filename, ActionState::Action_Strings(), false);

	// Hardcode default binds here
	static auto defaultBind= [&](const ActionState::ACTION_ENUM & bind, const KeyboardEvent::Key & key){
		if (m_config->m_configuration.find(bind) == m_config->m_configuration.end())
			m_config->m_configuration[bind] = (float)key;
	};
	defaultBind(ActionState::UI_UP, KeyboardEvent::W);
	defaultBind(ActionState::UI_DOWN, KeyboardEvent::S);
	defaultBind(ActionState::UI_LEFT, KeyboardEvent::A);
	defaultBind(ActionState::UI_RIGHT, KeyboardEvent::D);
	defaultBind(ActionState::FORWARD, KeyboardEvent::W);
	defaultBind(ActionState::BACKWARD, KeyboardEvent::S);
	defaultBind(ActionState::LEFT, KeyboardEvent::A);
	defaultBind(ActionState::RIGHT, KeyboardEvent::D);
	defaultBind(ActionState::JUMP, KeyboardEvent::SPACE);
	defaultBind(ActionState::RUN, KeyboardEvent::LEFT_SHIFT);
	defaultBind(ActionState::UI_ENTER, KeyboardEvent::ENTER);
	defaultBind(ActionState::UI_ESCAPE, KeyboardEvent::ESCAPE);
}

void InputBinding::save() {
	if (m_config->existsYet())
		m_config->saveConfig();
}

const Shared_Config & InputBinding::getBindings() const
{ 
	return m_config;
}
