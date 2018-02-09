#include "Entities\Entity.h"
#include "Entities\Components\Component.h"
#include "Systems\World\ECSmessage.h"
#include "Systems\World\Component_Factory.h"
#include "Systems\World\ECSmessanger.h"

void Entity::addComponent(char *type)
{
	m_component_handles.insert(std::pair<char*, vector<unsigned int>>(type, vector<unsigned int>()));
	m_component_handles[type].push_back(m_componentFactory->CreateComponent(type, m_ID).second);
}

Component * Entity::getComponent(const ECShandle & id)
{
	return m_componentFactory->GetComponent(id);
}

void Entity::SendMessage(const ECSmessage &message)
{
	// Forward message to all components
	m_ECSmessanger->SendMessage_ToComponents(message, m_component_handles);
}

void Entity::ReceiveMessage(const ECSmessage &message)
{
	// Forward message to all components
	SendMessage(message);
}

Entity::~Entity()
{
	for each (auto pair in m_component_handles)
		for each (auto id in pair.second) 
			m_componentFactory->DeleteComponent(ECShandle(pair.first, id));
}
