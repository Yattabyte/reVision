/*
	ECSmessanger

	- Handles delivery of messages between entities and components
*/

#pragma once
#ifndef ECSMESSANGER
#define ECSMESSANGER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\ECS\ECSmessage.h"
#include "Systems\ECS\ECSdefines.h"
#include <map>
#include <vector>

namespace ECSmessanger {
	DT_ENGINE_API void SendMessage_ToEntity(const ECSmessage &message, const ECShandle &target);
	DT_ENGINE_API void SendMessage_ToComponent(const ECSmessage &message, const ECShandle &target);
	DT_ENGINE_API void SendMessage_ToComponents(const ECSmessage &message, const ECShandle_map &targets);
}

#endif // ECSMESSANGER
