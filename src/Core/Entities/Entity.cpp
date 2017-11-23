#include "Entities\Entity.h"
#include "Entities\Components\Component.h"
#include "Systems\ECS\ECSmessage.h"
#include "Systems\ECS\ComponentFactory.h"
#include "Systems\ECS\ECSmessanger.h"

void Entity::addComponent(char *type)
{
	m_component_handles.insert(std::pair<char*, vector<unsigned int>>(type, vector<unsigned int>()));
	m_component_handles[type].push_back(ComponentFactory::CreateComponent(type, m_ID).second);
}

Component * Entity::getComponent(const ECShandle & id)
{
	return ComponentFactory::GetComponent(id);
}

void Entity::SendMessage(const ECSmessage &message)
{
	// Forward message to all components
	ECSmessanger::SendMessage_ToComponents(message, m_component_handles);
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
			ComponentFactory::DeleteComponent(ECShandle(pair.first, id));			
}
