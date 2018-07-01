#include "Systems/Input/InputBinding.h"
#include "Engine.h"


InputBinding::InputBinding(Engine * engine, const std::string & filename) 
{ 
	engine->createAsset(bindings, filename, ActionState::Action_Strings(), false); 
}

const Shared_Asset_Config & InputBinding::getBindings() const 
{ 
	return bindings; 
}
