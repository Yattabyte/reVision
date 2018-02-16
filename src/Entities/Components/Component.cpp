#include "Entities\Components\Component.h"
#include "Systems\World\ECSmessanger.h"
#include "Systems\World\ECSmessage.h"


void Component::receiveMessage(const ECSmessage & message)
{
	// Do something
}

bool Component::compareMSGSender(const ECSmessage & message)
{
	return (message.GetSenderID() == m_ID);
}
	
