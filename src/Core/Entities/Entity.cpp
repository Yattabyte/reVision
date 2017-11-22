#include "Entities\Entity.h"
#include "Entities\Components\Component.h"
#include "Systems\ECS\ECSmessage.h"
#include "Systems\ECS\ComponentFactory.h"

void Entity::addComponent(char *type)
{
	m_component_handles.insert(std::pair<char*, vector<unsigned int>>(type, vector<unsigned int>()));
	m_component_handles[type].push_back(ComponentFactory::CreateComponent(type, m_ID).second);
}

Component * Entity::getComponent(const ECSHandle & id)
{
	return ComponentFactory::GetComponent(id);
}

void Entity::SendMessage(ECSmessage &message)
{
	// Forward message to all components
	ComponentFactory::SendMessageToComponents(message, m_component_handles);
}

void Entity::ReceiveMessage(ECSmessage &message)
{
	// Forward message to all components
	SendMessage(message);
}

Entity::~Entity()
{
	for each (auto pair in m_component_handles)
		for each (auto id in pair.second) 
			ComponentFactory::DeleteComponent(ECSHandle(pair.first, id));			
}
