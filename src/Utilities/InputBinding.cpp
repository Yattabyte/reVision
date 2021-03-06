#include "Utilities/InputBinding.h"
#include "Utilities/ActionState.h"
#include "Modules/UI/KeyboardEvent.h"


InputBinding::~InputBinding()
{
	save();
}

InputBinding::InputBinding(Engine& engine) noexcept :
	m_engine(engine)
{
}

void InputBinding::loadFile(const std::string& filename)
{
	m_config = Shared_Config(m_engine, filename, ActionState::Action_Strings(), false);

	// Hard-code default binds here
	static auto defaultBind = [&](const ActionState::Action& bind, const KeyboardEvent::Key& key) {
		if (m_config->m_configuration.find(static_cast<unsigned int>(bind)) == m_config->m_configuration.end())
			m_config->m_configuration[static_cast<unsigned int>(bind)] = (float)key;
	};
	defaultBind(ActionState::Action::UI_UP, KeyboardEvent::Key::W);
	defaultBind(ActionState::Action::UI_DOWN, KeyboardEvent::Key::S);
	defaultBind(ActionState::Action::UI_LEFT, KeyboardEvent::Key::A);
	defaultBind(ActionState::Action::UI_RIGHT, KeyboardEvent::Key::D);
	defaultBind(ActionState::Action::FORWARD, KeyboardEvent::Key::W);
	defaultBind(ActionState::Action::BACKWARD, KeyboardEvent::Key::S);
	defaultBind(ActionState::Action::LEFT, KeyboardEvent::Key::A);
	defaultBind(ActionState::Action::RIGHT, KeyboardEvent::Key::D);
	defaultBind(ActionState::Action::JUMP, KeyboardEvent::Key::SPACE);
	defaultBind(ActionState::Action::RUN, KeyboardEvent::Key::LEFT_SHIFT);
	defaultBind(ActionState::Action::UI_ENTER, KeyboardEvent::Key::ENTER);
	defaultBind(ActionState::Action::UI_ESCAPE, KeyboardEvent::Key::ESCAPE);
}

void InputBinding::save()
{
	if (m_config->ready())
		m_config->saveConfig();
}

Shared_Config InputBinding::getBindings() const noexcept
{
	return m_config;
}