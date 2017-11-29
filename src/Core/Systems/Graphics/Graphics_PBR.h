/*
	Graphics_PBR

	- A system whos job is to render a given scene from a particular viewing perspective
	- Aims for high realism, uses physically based rendering techniques
*/



#pragma once
#ifndef SYSTEM_GRAPHICS_PBR
#define SYSTEM_GRAPHICS_PBR
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\System_Interface.h"
#include "Rendering\Scenes\PBR\Geometry_Buffer.h"
#include "Rendering\Scenes\PBR\Lighting_Buffer.h"
#include "Rendering\Visibility_Token.h"

class Camera;
class DT_ENGINE_API System_Graphics_PBR : public System
{
public: 
	~System_Graphics_PBR();
	System_Graphics_PBR(Camera *engineCamera);

	// Render a frame
	void Update(const float &deltaTime);


private:
	void RegenerationPass(const Visibility_Token &vis_token);
	void GeometryPass(const Visibility_Token &vis_token);
	void LightingPass(const Visibility_Token &vis_token);
	void FinalPass(const Visibility_Token &vis_token);

	Camera *m_engineCamera;
	Geometry_Buffer m_gbuffer;
	Lighting_Buffer m_lbuffer;
	float screen_width, screen_height;
};

#endif // SYSTEM_GRAPHICS_PBR