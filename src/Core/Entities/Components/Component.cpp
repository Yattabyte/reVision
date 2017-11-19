#include "Entities\Components\Component.h"
#include "Systems\ECS\ComponentFactory.h"
#include "Systems\ECS\EntityFactory.h"
#include "Systems\ECS\ECSMessage.h"

void Component::SendMessage(ECSMessage * message)
{
	// Forward Message to parent
	EntityFactory::SendMessageToEntity(message, m_parentID);
}

void Component::ReceiveMessage(ECSMessage * message)
{
	// Do something
}
	
