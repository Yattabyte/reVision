/*
	Visibility_Token

	- An element that defines where and how a scene should be viewed
*/

#pragma once
#ifndef VISIBILITY_TOKEN
#define VISIBILITY_TOKEN
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Entities\Components\Component.h"
#include "Systems\ECS\ECSdefines.h"
#include <map>
#include <vector>

typedef std::map<char*, std::vector<Component*>, cmp_str> Visibility_Token;

#endif // VISIBILITY_TOKEN