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
#include "Systems\Graphics\Geometry_Buffer.h"
#include "Systems\Graphics\Lighting_Buffer.h"
#include "Rendering\Visibility_Token.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Texture.h"

class Engine_Package;

class Camera;
class DT_ENGINE_API System_Graphics_PBR : public System
{
public: 
	~System_Graphics_PBR();
	System_Graphics_PBR();
	void Initialize(Engine_Package * enginePackage);

	// Render a frame
	void Update(const float &deltaTime);
	
private:
	void RegenerationPass(const Visibility_Token &vis_token);
	void GeometryPass(const Visibility_Token &vis_token);
	void LightingPass(const Visibility_Token &vis_token);
	void FinalPass(const Visibility_Token &vis_token);

	Geometry_Buffer m_gbuffer;
	Lighting_Buffer m_lbuffer;
	Shared_Asset_Shader m_shaderGeometry, m_shaderGeometryShadow, m_shaderLighting, m_shaderTest;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	bool m_quadLoaded;
	Shared_Asset_Texture m_texture;
};

#endif // SYSTEM_GRAPHICS_PBR