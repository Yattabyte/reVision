#include "Entities\Components\Component.h"
#include "Systems\ECS\ComponentFactory.h"
#include "Systems\ECS\EntityFactory.h"
#include "Systems\ECS\ECSmessage.h"

void Component::SendMessage(ECSmessage * message)
{
	// Forward Message to parent
	EntityFactory::SendMessageToEntity(message, m_parentID);
}

void Component::ReceiveMessage(ECSmessage * message)
{
	// Do something
}
	
