#include "Modules/ECS/ECS_M.h"


void ECS_Module::initialize(Engine* engine)
{
}

void ECS_Module::deinitialize()
{
}

void ECS_Module::frameTick(const float& deltaTime)
{
}

void ECS_Module::updateSystems(ecsSystemList& systems, const float& deltaTime)
{
	m_world.updateSystems(systems, deltaTime);
}

void ECS_Module::updateSystem(ecsBaseSystem* system, const float& deltaTime)
{
	m_world.updateSystem(system, deltaTime);
}

void ECS_Module::updateSystem(const std::shared_ptr<ecsBaseSystem>& system, const float& deltaTime)
{
	m_world.updateSystem(system, deltaTime);
}

void ECS_Module::updateSystem(const float& deltaTime, const std::vector<ComponentID>& types, const std::vector<ecsBaseSystem::RequirementsFlag>& flags, const std::function<void(const float&, const std::vector<std::vector<ecsBaseComponent*>>&)>& func)
{
	m_world.updateSystem(deltaTime, types, flags, func);
}