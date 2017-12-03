#include "Systems\World\ECSmessanger.h"
#include "Systems\World\Entity_Factory.h"
#include "Systems\World\Component_Factory.h"

ECSmessanger::~ECSmessanger()
{
}

ECSmessanger::ECSmessanger(Entity_Factory * entityFactory, Component_Factory * componentFactory) : 
	m_entityFactory(entityFactory), 
	m_componentFactory(componentFactory)
{
}

void ECSmessanger::SendMessage_ToEntity(const ECSmessage & message, const ECShandle & target)
{
	auto entity = m_entityFactory->GetEntity(target);
	if (entity == nullptr) return;
	entity->ReceiveMessage(message);
}

void ECSmessanger::SendMessage_ToComponent(const ECSmessage & message, const ECShandle & target)
{
	auto component = m_componentFactory->GetComponent(target);
	if (component == nullptr) return;
	component->ReceiveMessage(message);
}

void ECSmessanger::SendMessage_ToComponents(const ECSmessage & message, const ECShandle_map & targets)
{
	for each (const auto pair in targets) 
		for each (const auto componentID in pair.second) 
			SendMessage_ToComponent(message, ECShandle(pair.first, componentID));
}
