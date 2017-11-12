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

#include "Entities\Geometry.h"
#include "Entities\Light.h"
#include <map>
#include <vector>

using namespace std;

class Visibility_Token
{
public:
	/*************
	----Common----
	*************/

	~Visibility_Token() {};
	Visibility_Token() {};


	/****************
	----Variables----
	****************/

	map<int, vector<Geometry*>> visible_geometry;
	map<int, vector<Light*>> visible_lights, visible_shadow_lights;
};

#endif // VISIBILITY_TOKEN