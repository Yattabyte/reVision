#include "Systems\World\ECS\ECSmessenger.h"
#include "Systems\World\ECS\Entity_Factory.h"
#include "Systems\World\ECS\Component_Factory.h"


ECSmessenger::~ECSmessenger()
{
}

ECSmessenger::ECSmessenger(Entity_Factory * entityFactory, Component_Factory * componentFactory) : 
	m_entityFactory(entityFactory), 
	m_componentFactory(componentFactory)
{
}

void ECSmessenger::SendMessage_ToEntity(const ECSmessage & message, const ECShandle & target)
{
	auto entity = m_entityFactory->getEntity(target);
	if (entity == nullptr) return;
	entity->receiveMessage(message);
}

void ECSmessenger::SendMessage_ToComponent(const ECSmessage & message, const ECShandle & target)
{
	auto component = m_componentFactory->getComponent(target);
	if (component == nullptr) return;
	component->receiveMessage(message);
}

void ECSmessenger::SendMessage_ToComponents(const ECSmessage & message, const VectorMap<unsigned int> &targets)
{
	for each (const auto pair in targets) 
		for each (const auto componentID in pair.second) 
			SendMessage_ToComponent(message, ECShandle(pair.first, componentID));
}
