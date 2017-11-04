/*
	Test_Scene

	- A rendering scene for debugging and testing out early engine rendering functions
*/

#include "Rendering\Scene.h"

#pragma once
#ifndef TEST_SCENE
#define TEST_SCENE
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define DELTA_CORE_API __declspec(dllimport)
#endif

class Test_Scene : public Scene
{
public: 
	DELTA_CORE_API ~Test_Scene();
	DELTA_CORE_API Test_Scene();
	DELTA_CORE_API virtual void RenderFrame();
};

#endif // TEST_SCENE