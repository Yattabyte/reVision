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
class VisualFX;
class DT_ENGINE_API Lighting_Buffer
{
public:
	/*************
	----Common----
	*************/

	~Lighting_Buffer();
	Lighting_Buffer();
	void Initialize(Engine_Package *enginePackage, VisualFX *visualFX, const GLuint &depthStencil);


	/********************************
	----Lighting_Buffer Functions----
	********************************/

	// Binds and clears out all the texture rendertargets in this framebuffer
	void Clear();
	// Binds the framebuffer and its rendertargets for writing
	void BindForWriting();
	// Binds the framebuffer and its rendertargets for reading
	void BindForReading();
	// Change the size of the framebuffer object
	void Resize(const vec2 & size);
	// Set the strength of the bloom effect
	void SetBloomStrength(const int &strength);
	// Apply blur filter to bloom attachment, because its useless otherwise
	void ApplyBloom();


	/****************
	----Variables----
	****************/

	enum LBUFFER_TEXTURE_TYPE {
		LBUFFER_TEXTURE_TYPE_SCENE,
		LBUFFER_TEXTURE_TYPE_OVERBRIGHT,
		LBUFFER_NUM_TEXTURES
	};
	GLuint m_fbo;
	GLuint m_textures[LBUFFER_NUM_TEXTURES], m_texturesGB[2];
	GLuint m_depth_stencil; // Donated by the geometry buffer

private:
	Engine_Package *m_enginePackage;
	VisualFX *m_visualFX;
	Callback_Container *m_widthChangeCallback, *m_heightChangeCallback, *m_bloomStrengthChangeCallback;
	vec2 m_renderSize;
	int m_bloomStrength;
	bool m_Initialized;
};

#endif // LIGHTING_BUFFER