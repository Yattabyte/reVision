#include "Systems\ECS\ECSmessanger.h"
#include "Systems\ECS\EntityFactory.h"
#include "Systems\ECS\ComponentFactory.h"

void ECSmessanger::SendMessage_ToEntity(const ECSmessage & message, const ECShandle & target)
{
	auto entity = EntityFactory::GetEntity(target);
	if (entity == nullptr) return;
	entity->ReceiveMessage(message);
}

void ECSmessanger::SendMessage_ToComponent(const ECSmessage & message, const ECShandle & target)
{
	auto component = ComponentFactory::GetComponent(target);
	if (component == nullptr) return;
	component->ReceiveMessage(message);
}

void ECSmessanger::SendMessage_ToComponents(const ECSmessage & message, const ECShandle_map & targets)
{
	for each (const auto pair in targets) 
		for each (const auto componentID in pair.second) 
			SendMessage_ToComponent(message, ECShandle(pair.first, componentID));
}
