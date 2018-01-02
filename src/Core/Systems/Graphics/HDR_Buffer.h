/*
	HDR_Buffer

	- A specialized frame buffer
	- Color corrects and joins lighting results together
*/

#pragma once
#ifndef HDR_BUFFER
#define HDR_BUFFER
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

class DT_ENGINE_API HDR_Buffer
{
public:
	/*************
	----Common----
	*************/

	~HDR_Buffer();
	HDR_Buffer();
	void Initialize(Engine_Package *enginePackage);


	/***************************
	----HDR_Buffer Functions----
	***************************/

	// Binds and clears out all the texture rendertargets in this framebuffer
	void Clear();
	// Binds the framebuffer and its rendertargets for writing
	void BindForWriting();
	// Binds the framebuffer and its rendertargets for reading
	void BindForReading();
	// Change the size of the framebuffer object
	void Resize(const vec2 & size);


	/****************
	----Variables----
	****************/

	GLuint m_fbo;
	GLuint m_texture;

private:
	Engine_Package *m_enginePackage;
	Callback_Container *m_widthChangeCallback, *m_heightChangeCallback;
	bool m_Initialized;
};

#endif // HDR_BUFFER