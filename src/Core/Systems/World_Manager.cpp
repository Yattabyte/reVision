#include "Systems\World_Manager.h"
#include "Systems\Factories\ComponentFactory.h"
#include "Systems\Factories\EntityFactory.h"
#include <algorithm>

#include "Entities\Prop.h"
void World_Manager::startup()
{
	ComponentFactory::Startup();
	EntityFactory::Startup();
}

void World_Manager::shutdown()
{
	UnloadWorld();
}

void World_Manager::LoadWorld()
{

}

void World_Manager::UnloadWorld()
{
	
}
