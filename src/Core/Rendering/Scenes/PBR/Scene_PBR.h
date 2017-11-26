/*
	Scene_PBR

	- A rendering scene that attempts to render things as physically as possible (aka PBR)
*/

#include "Rendering\Scenes\Scene.h"
#include "Rendering\Scenes\PBR\Geometry_Buffer.h"
#include "Rendering\Scenes\PBR\Lighting_Buffer.h"
#include "Rendering\Visibility_Token.h"

#pragma once
#ifndef TEST_SCENE
#define TEST_SCENE
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define DT_ENGINE_API __declspec(dllimport)
#endif

class Scene_PBR : public Scene
{
public: 
	DT_ENGINE_API ~Scene_PBR();
	DT_ENGINE_API Scene_PBR();
	DT_ENGINE_API virtual void RenderFrame(Camera *);

protected:
	void RegenerationPass(const Visibility_Token &vis_token);
	void GeometryPass(const Visibility_Token &vis_token);
	void LightingPass(const Visibility_Token &vis_token);
	void FinalPass(const Visibility_Token &vis_token);

private:
	Geometry_Buffer m_gbuffer;
	Lighting_Buffer m_lbuffer;
};

#endif // TEST_SCENE