/*
	Scene

	- Rendering scenes direct the process for rendering a frame
	- Abstract class
*/

#pragma once
#ifndef SCENE
#define SCENE
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define DT_ENGINE_API __declspec(dllimport)
#endif

class Camera;
class Scene
{
public: 
	~Scene() {};
	Scene() {};
	virtual void RenderFrame(Camera *) {};
};

#endif // SCENE