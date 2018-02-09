#include "Entities\Components\Component.h"
#include "Systems\World\ECSmessanger.h"
#include "Systems\World\ECSmessage.h"

void Component::SendMessage(const ECSmessage &message)
{
	// Forward Message to parent
	m_ECSmessanger->SendMessage_ToEntity(message, m_parentID);
}

void Component::ReceiveMessage(const ECSmessage &message)
{
	// Do something
}

bool Component::Am_I_The_Sender(const ECSmessage & message)
{
	return (message.GetSenderID() == m_ID);
}
	
