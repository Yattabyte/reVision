/*
	VisualFX

	- Techniques for applying various visual effects to an image/frame
*/
/*
#pragma once
#ifndef VISUALFX
#define VISUALFX
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define CF_MIP_LODS 6
#define CF_MIP_SIZE 1024
#define GLEW_STATIC

#include "GL\glew.h"
#include "GLM\common.hpp"

using namespace glm;

namespace VisualFX {
	DT_ENGINE_API void Initialize();
	DT_ENGINE_API void APPLY_FX_CUBE_FILTER(const GLuint &sourceTexture, const GLuint & destinationTexture, const float &size);
	DT_ENGINE_API void APPLY_FX_GAUSSIAN_BLUR(const GLuint & desiredTexture, const GLuint *flipTextures, const vec2 &size, const int &amount);
	DT_ENGINE_API void APPLY_FX_GAUSSIAN_BLUR_ALPHA(const GLuint & desiredTexture, const GLuint *flipTextures, const vec2 &size, const int &amount);
};
#endif // VISUALFX*/