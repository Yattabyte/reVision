#include "Systems\World\ECS\Entities\Entity.h"
#include "Systems\World\ECS\Components\Component.h"
#include "Systems\World\ECS\Component_Factory.h"


void EntityCreator::destroy(Entity * entity)
{
	delete entity;
}

Component * EntityCreator::makeComponent(const char * type)
{
	return m_componentFactory->createComponent(type);
}

void EntityCreator::unMakeComponent(Component * component)
{
	m_componentFactory->deleteComponent(component);
}