#include "Modules/ECS/ECS_M.h"


void ECS_Module::initialize(Engine*)
{
}

void ECS_Module::deinitialize()
{
}

void ECS_Module::updateSystems(ecsSystemList& systems, ecsWorld& world, const float& deltaTime)
{
	world.updateSystems(systems, deltaTime);
}

void ECS_Module::updateSystem(ecsBaseSystem* system, ecsWorld& world, const float& deltaTime)
{
	world.updateSystem(system, deltaTime);
}

void ECS_Module::updateSystem(const std::shared_ptr<ecsBaseSystem>& system, ecsWorld& world, const float& deltaTime)
{
	world.updateSystem(system, deltaTime);
}

void ECS_Module::updateSystem(const float& deltaTime, ecsWorld& world, const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes, const std::function<void(const float&, const std::vector<std::vector<ecsBaseComponent*>>&)>& func)
{
	world.updateSystem(deltaTime, componentTypes, func);
}