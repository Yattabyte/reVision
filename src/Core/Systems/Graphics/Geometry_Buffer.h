/*
	Geometry_Buffer

	- A specialized frame buffer
	- The backbone of our deferred renderer
	- Geometry renders into it, storing surface values like albedo, normal, specular, and depth
	- Values stored in view space, depth can get converted into view position -> world position
*/

#pragma once
#ifndef GEOMETRY_BUFFER
#define GEOMETRY_BUFFER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLEW_STATIC

#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "GL\glew.h"
#include "glm\glm.hpp"

using namespace glm;

class VisualFX;
class DT_ENGINE_API Geometry_Buffer
{
public:
	/*************
	----Common----
	*************/

	~Geometry_Buffer();
	Geometry_Buffer();
	void Initialize(const vec2 &size, VisualFX *visualFX);


	/********************************
	----Geometry_Buffer Functions----
	********************************/

	// Binds and clears out all the texture rendertargets in this framebuffer
	void Clear();
	// Binds the framebuffer and its rendertargets for writing
	void BindForWriting();
	// Binds the framebuffer and its rendertargets for reading
	void BindForReading();
	// Resets the state and ensures its rendertargets are attached
	void End();
	// Change the size of the framebuffer object
	void Resize(const vec2 & size);
	// Generate ambient occlusion for the frame
	void ApplyAO();


	/****************
	----Variables----
	****************/

	enum GBUFFER_TEXTURE_TYPE {
		GBUFFER_TEXTURE_TYPE_IMAGE,
		GBUFFER_TEXTURE_TYPE_VIEWNORMAL,
		GBUFFER_TEXTURE_TYPE_SPECULAR,
		GBUFFER_NUM_TEXTURES
	};
	GLuint m_fbo, m_noiseID;
	GLuint m_textures[GBUFFER_NUM_TEXTURES], m_texturesGB[2], m_depth_stencil;

private:
	VisualFX *m_visualFX;
	Shared_Asset_Shader m_shaderSSAO;	
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_vao_Quad;
	vec2 m_renderSize;
	bool m_Initialized;
	void *m_observer;
};

#endif // GEOMETRY_BUFFER