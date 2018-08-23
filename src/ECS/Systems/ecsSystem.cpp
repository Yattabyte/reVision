#include "ECS\Systems\ecsSystem.h"


const bool BaseECSSystem::isValid() const
{
	for (unsigned int i = 0; i < componentFlags.size(); ++i) {
		if ((componentFlags[i] & BaseECSSystem::FLAG_OPTIONAL) == 0)
			return true;
	}
	return false;
}

const bool ECSSystemList::removeSystem(BaseECSSystem * system)
{
	for (unsigned int i = 0; i < systems.size(); ++i) {
		if (system == systems[i]) {
			systems.erase(systems.begin() + i);
			return true;
		}
	}
	return false;
}