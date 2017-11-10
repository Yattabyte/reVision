/*
	Scene

	- Rendering scenes direct the process for rendering a frame
	- Abstract class
*/

#pragma once
#ifndef SCENE
#define SCENE
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define DELTA_CORE_API __declspec(dllimport)
#endif

class Scene
{
public: 
	~Scene() {};
	Scene() {};
	virtual void RenderFrame() {};
};

#endif // SCENE