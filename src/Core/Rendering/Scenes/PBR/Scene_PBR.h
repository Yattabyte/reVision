/*
	Scene_PBR

	- A rendering scene that attempts to render things as physically as possible (aka PBR)
*/

#include "Rendering\Scenes\Scene.h"
#include "Rendering\Scenes\PBR\Gbuffer.h"

#pragma once
#ifndef TEST_SCENE
#define TEST_SCENE
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define DELTA_CORE_API __declspec(dllimport)
#endif

class Scene_PBR : public Scene
{
public: 
	DELTA_CORE_API ~Scene_PBR();
	DELTA_CORE_API Scene_PBR();
	DELTA_CORE_API virtual void RenderFrame();

private:
	GBuffer m_gbuffer;
};

#endif // TEST_SCENE