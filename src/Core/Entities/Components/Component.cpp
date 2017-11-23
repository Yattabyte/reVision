#include "Entities\Components\Component.h"
#include "Systems\ECS\ECSmessanger.h"
#include "Systems\ECS\ECSmessage.h"

void Component::SendMessage(const ECSmessage &message)
{
	// Forward Message to parent
	ECSmessanger::SendMessage_ToEntity(message, m_parentID);
}

void Component::ReceiveMessage(const ECSmessage &message)
{
	// Do something
}

bool Component::Am_I_The_Sender(const ECSmessage & message)
{
	return (message.GetSenderID() == m_ID);
}
	
