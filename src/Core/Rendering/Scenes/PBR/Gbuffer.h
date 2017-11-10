/*
	GBuffer

	- A specialized frame buffer
	- The backbone of our deferred renderer
	- Has several render targets such as albedo, normal, and specular, all in viewing space
*/

#pragma once
#ifndef GBUFFER
#define GBUFFER
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif
#define GLEW_STATIC

#include "GL\glew.h"


class GBuffer
{
public:
	/*************
	----Common----
	*************/

	DELTA_CORE_API ~GBuffer();
	DELTA_CORE_API GBuffer();


	/************************
	----GBuffer Functions----
	************************/

	// Binds and clears out all the texture rendertargets in this framebuffer
	DELTA_CORE_API void Clear();
	// Binds the framebuffer and its rendertargets for writing
	DELTA_CORE_API void BindForWriting();
	// Binds the framebuffer and its rendertargets for reading
	DELTA_CORE_API void BindForReading();
	// Resets the state and ensures its rendertargets are attached
	DELTA_CORE_API void End();


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

#endif // GBUFFER