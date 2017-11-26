/*
	ECSmessanger

	- Handles delivery of messages between entities and components
*/

#pragma once
#ifndef ECSMESSANGER
#define ECSMESSANGER
#ifdef	ENGINE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Systems\ECS\ECSmessage.h"
#include "Systems\ECS\ECSdefines.h"
#include <map>
#include <vector>

namespace ECSmessanger {
	DELTA_CORE_API void SendMessage_ToEntity(const ECSmessage &message, const ECShandle &target);
	DELTA_CORE_API void SendMessage_ToComponent(const ECSmessage &message, const ECShandle &target);
	DELTA_CORE_API void SendMessage_ToComponents(const ECSmessage &message, const ECShandle_map &targets);
}

#endif // ECSMESSANGER
