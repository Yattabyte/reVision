/*
	Lighting_Buffer

	- A specialized frame buffer
	- Lights render into it, reading from the Geometry buffer	
*/

#pragma once
#ifndef LIGHTING_BUFFER
#define LIGHTING_BUFFER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLEW_STATIC

#include "GL\glew.h"
#include "glm\glm.hpp"

using namespace glm;

class Engine_Package;
class Callback_Container;

class Lighting_Buffer
{
public:
	/*************
	----Common----
	*************/

	DT_ENGINE_API ~Lighting_Buffer();
	DT_ENGINE_API Lighting_Buffer(Engine_Package *package, const GLuint &m_depth_stencil);


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

	enum LBUFFER_TEXTURE_TYPE {
		LBUFFER_TEXTURE_TYPE_SCENE,
		LBUFFER_TEXTURE_TYPE_OVERBRIGHT,
		LBUFFER_NUM_TEXTURES
	};
	GLuint m_fbo;
	GLuint m_textures[LBUFFER_NUM_TEXTURES];
	GLuint m_depth_stencil; // Donated by the geometry buffer

private:
	Engine_Package *m_enginePackage;
	Callback_Container *m_widthChangeCallback, *m_heightChangeCallback;
};

#endif // LIGHTING_BUFFER