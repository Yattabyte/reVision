/*
	Visibility_Token

	- An element that defines where and how a scene should be viewed
*/

#pragma once
#ifndef VISIBILITY_TOKEN
#define VISIBILITY_TOKEN
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Entities\Components\Component.h"
#include <map>
#include <vector>

struct cmp_str { bool operator()(const char *a, const char *b) const { return std::strcmp(a, b) < 0; } };

typedef std::map<char*, std::vector<Component*>, cmp_str> Visibility_Token;

#endif // VISIBILITY_TOKEN