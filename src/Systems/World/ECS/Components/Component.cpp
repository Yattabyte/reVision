#include "Systems\World\ECS\Components\Component.h"
#include "Systems\World\ECS\ECSmessenger.h"
#include "Systems\World\ECS\ECSmessage.h"


void Component::receiveMessage(const ECSmessage & message)
{
	// Do something
}

bool Component::compareMSGSender(const ECSmessage & message)
{
	return (message.GetSenderID() == m_ID);
}
	
