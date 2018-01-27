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
#define MAX_KERNEL_SIZE 128 // Don't manipulate this, set the usable to a different value < than this

#include "Systems\System_Interface.h"
#include "Systems\Graphics\Geometry_Buffer.h"
#include "Systems\Graphics\Lighting_Buffer.h"
#include "Systems\Graphics\HDR_Buffer.h"
#include "Systems\Graphics\VisualFX.h"
#include "Systems\World\Visibility_Token.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Cubemap.h"

struct Renderer_Attribs
{
	vec4 kernel[MAX_KERNEL_SIZE];
	float m_ssao_radius;
	int m_ssao_strength, m_aa_samples;
	int m_ssao;
};

class Engine_Package;
class Callback_Container;
class Camera;
class DT_ENGINE_API System_Graphics_PBR : public System
{
public: 
	~System_Graphics_PBR();
	System_Graphics_PBR();
	void Initialize(Engine_Package * enginePackage);

	void GenerateKernal();
	void SetSSAO(const bool &ssao);
	void SetSSAOSamples(const int &samples);
	void SetSSAOStrength(const int &strength);
	void SetSSAORadius(const float &radius);
	void Resize(const vec2 & size);

	// Render a frame
	void Update(const float &deltaTime);
	
private:
	void RegenerationPass(const Visibility_Token &vis_token);
	void GeometryPass(const Visibility_Token &vis_token);
	void SkyPass();
	void LightingPass(const Visibility_Token &vis_token);
	void HDRPass();
	void FinalPass();

	Renderer_Attribs m_attribs;
	GLuint m_attribID;
	Callback_Container *m_ssaoCallback, *m_ssaoSamplesCallback, *m_ssaoStrengthCallback, *m_ssaoRadiusCallback, *m_bloomStrengthChangeCallback, *m_widthChangeCallback, *m_heightChangeCallback;
	vec2 m_renderSize;
	VisualFX m_visualFX;
	Geometry_Buffer m_gbuffer;
	Lighting_Buffer m_lbuffer;
	HDR_Buffer m_hdrbuffer;
	Shared_Asset_Shader m_shaderGeometry, m_shaderShadowDir, m_shaderShadowSpot, m_shaderDirectional, m_shaderSpot, m_shaderSky, m_shaderHDR, m_shaderFXAA;
	Shared_Asset_Primitive m_shapeQuad, m_shapeCone;
	GLuint m_quadVAO, m_coneVAO;
	Shared_Asset_Cubemap m_textureSky;
	void* m_QuadObserver, *m_ConeObserver;
};

#endif // SYSTEM_GRAPHICS_PBR