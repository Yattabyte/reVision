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
#include "Systems\Graphics\HDR_Buffer.h"
#include "Rendering\Visibility_Token.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Cubemap.h"

class Engine_Package;

class Camera;
class Primitive_Observer;
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
	void SkyPass();
	void LightingPass(const Visibility_Token &vis_token);
	void HDRPass();
	void FinalPass();

	Geometry_Buffer m_gbuffer;
	Lighting_Buffer m_lbuffer;
	HDR_Buffer m_hdrbuffer;
	Shared_Asset_Shader m_shaderGeometry, m_shaderGeometryShadow, m_shaderLighting, m_shaderSky, m_shaderHDR;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	Shared_Asset_Cubemap m_textureSky;
	shared_ptr<Primitive_Observer> m_observer;
};

class DT_ENGINE_API Primitive_Observer : Asset_Observer
{
public:
	Primitive_Observer(Shared_Asset_Primitive &asset, const GLuint vao) : Asset_Observer(asset.get()), m_vao_id(vao), m_asset(asset) {};
	virtual ~Primitive_Observer() { m_asset->RemoveObserver(this); };
	virtual void Notify_Finalized();

	GLuint m_vao_id;
	Shared_Asset_Primitive m_asset;
};

#endif // SYSTEM_GRAPHICS_PBR