#include "Utilities/InputBinding.h"
#include "Engine.h"


InputBinding::InputBinding(Engine * engine) : m_engine(engine) {}

void InputBinding::loadFile(const std::string & filename)
{
	m_config = Shared_Config(m_engine, filename, ActionState::Action_Strings(), false);
}

const Shared_Config & InputBinding::getBindings() const
{ 
	return m_config;
}
