/*
	VisualFX

	- Techniques for applying various visual effects to an image/frame
*/
/*
#pragma once
#ifndef VISUALFX
#define VISUALFX
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif
#define CF_MIP_LODS 6
#define CF_MIP_SIZE 1024
#define GLEW_STATIC

#include "GL\glew.h"
#include "GLM\common.hpp"

using namespace glm;

namespace VisualFX {
	DELTA_CORE_API void Initialize();
	DELTA_CORE_API void APPLY_FX_CUBE_FILTER(const GLuint &sourceTexture, const GLuint & destinationTexture, const float &size);
	DELTA_CORE_API void APPLY_FX_GAUSSIAN_BLUR(const GLuint & desiredTexture, const GLuint *flipTextures, const vec2 &size, const int &amount);
	DELTA_CORE_API void APPLY_FX_GAUSSIAN_BLUR_ALPHA(const GLuint & desiredTexture, const GLuint *flipTextures, const vec2 &size, const int &amount);
};
#endif // VISUALFX*/