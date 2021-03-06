#include "Modules/ECS/ECS_M.h"


ECS_Module::ECS_Module(Engine& engine) noexcept :
	Engine_Module(engine)
{
}

void ECS_Module::initialize() noexcept
{
}

void ECS_Module::deinitialize() noexcept
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