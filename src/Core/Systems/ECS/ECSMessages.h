/*
	ECS_Messages

	- All of the types of messages can be stored here
*/

#pragma once
#ifndef ECS_MESSAGES
#define ECS_MESSAGES
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#define EXPIMP_TEMPLATE
#else
#define	DELTA_CORE_API __declspec(dllimport)
#define EXPIMP_TEMPLATE extern
#endif

#include "Systems\ECS\ECSmessage.h"
#include "Utilities\Transform.h"
#include <string>

using namespace std;
using namespace glm;


#endif // ECS_MESSAGES