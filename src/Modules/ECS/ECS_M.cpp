#include "Modules/ECS/ECS_M.h"


void ECS_Module::initialize(Engine*) noexcept
{
}

void ECS_Module::deinitialize() noexcept
{
}

void ECS_Module::updateSystems(ecsSystemList& systems, ecsWorld& world, const float& deltaTime) noexcept
{
	world.updateSystems(systems, deltaTime);
}

void ECS_Module::updateSystem(ecsBaseSystem* system, ecsWorld& world, const float& deltaTime) noexcept
{
	world.updateSystem(system, deltaTime);
}

void ECS_Module::updateSystem(const std::shared_ptr<ecsBaseSystem>& system, ecsWorld& world, const float& deltaTime) noexcept
{
	world.updateSystem(system, deltaTime);
}

void ECS_Module::updateSystem(const float& deltaTime, ecsWorld& world, const std::vector<std::pair<ComponentID, ecsBaseSystem::RequirementsFlag>>& componentTypes, const std::function<void(const float&, const std::vector<std::vector<ecsBaseComponent*>>&)>& func) noexcept
{
	world.updateSystem(deltaTime, componentTypes, func);
}