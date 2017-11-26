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

#include "GL\glew.h"
#include "glm\glm.hpp"

using namespace glm;

class Geometry_Buffer
{
public:
	/*************
	----Common----
	*************/

	DT_ENGINE_API ~Geometry_Buffer();
	DT_ENGINE_API Geometry_Buffer();


	/************************
	----Geometry_Buffer Functions----
	************************/

	// Binds and clears out all the texture rendertargets in this framebuffer
	DT_ENGINE_API void Clear();
	// Binds the framebuffer and its rendertargets for writing
	DT_ENGINE_API void BindForWriting();
	// Binds the framebuffer and its rendertargets for reading
	DT_ENGINE_API void BindForReading();
	// Resets the state and ensures its rendertargets are attached
	DT_ENGINE_API void End();
	// Change the size of the framebuffer object
	DT_ENGINE_API void Resize(const vec2 & size);


	/****************
	----Variables----
	****************/

	enum GBUFFER_TEXTURE_TYPE {
		GBUFFER_TEXTURE_TYPE_IMAGE,
		GBUFFER_TEXTURE_TYPE_VIEWNORMAL,
		GBUFFER_TEXTURE_TYPE_SPECULAR,
		GBUFFER_NUM_TEXTURES
	};
	GLuint m_fbo;
	GLuint m_textures[GBUFFER_NUM_TEXTURES], m_depth_stencil;
};

#endif // GEOMETRY_BUFFER