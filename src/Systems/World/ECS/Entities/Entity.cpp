#include "Systems\World\ECS\Entities\Entity.h"
#include "Systems\World\ECS\Components\Component.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\World\ECS\Component_Factory.h"
#include "Systems\World\ECS\ECSmessenger.h"


void Entity::addComponent(const char * type)
{
	m_component_handles.insert(type);
	m_component_handles[type].push_back(m_componentFactory->createComponent(type, m_ID).second);
}

Component * Entity::getComponent(const ECShandle & id)
{
	return m_componentFactory->getComponent(id);
}

void Entity::receiveMessage(const ECSmessage & message)
{
	// Forward message to all components
	m_ECSmessenger->SendMessage_ToComponents(message, m_component_handles);
}

Entity::~Entity()
{
	for each (auto pair in m_component_handles)
		for each (auto id in pair.second) 
			m_componentFactory->deleteComponent(ECShandle(pair.first, id));
}
