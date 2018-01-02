/*
	VisualFX

	- An effects class that manipulates images such as applying gaussian blur filters
*/

#pragma once
#ifndef VISUALFX
#define VISUALFX
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

class Engine_Package;
class DT_ENGINE_API VisualFX
{
public:
	/*************
	----Common----
	*************/

	~VisualFX();
	VisualFX();
	void Initialize(Engine_Package *enginePackage);
	

	/*************************
	----VisualFX Functions----
	*************************/
	
	//void applyCubeFilter(const GLuint &sourceTexture, const GLuint & destinationTexture, const float &size);
	void applyGaussianBlur(const GLuint & desiredTexture, const GLuint *flipTextures, const vec2 &size, const int &amount);
	//void applyGaussianBlur_Alpha(const GLuint & desiredTexture, const GLuint *flipTextures, const vec2 &size, const int &amount);


private:
	void Initialize_CubeFilter();
	void Initialize_GausianBlur();

	bool m_Initialized;
	Engine_Package *m_enginePackage;
	Shared_Asset_Primitive m_shapeQuad;
	void* m_observer;
	Shared_Asset_Shader m_shaderGB, m_shaderGB_A, m_shaderCF;
	GLuint m_vao_Quad, m_fbo_GB;
};

#endif // VISUALFX