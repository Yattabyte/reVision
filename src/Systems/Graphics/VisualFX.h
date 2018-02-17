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
class EnginePackage;


/**
 * A utility class that applies graphical effects/filters to images, such as Gaussian blur.
 **/
class DT_ENGINE_API VisualFX
{
public:
	// (de)Constructors
	/** Destructor. */
	~VisualFX();
	/** Constructor. */
	VisualFX();

	
	// Methods
	/** Initialize this utility and all its filters.
	 * @param	enginePackage	the engine package*/
	void initialize(EnginePackage * enginePackage);	
	/** Apply a Gaussian blur filter to the desired texture.
	 * @note	requires two other textures to flip between to apply the filter
	 * @param	desiredTexture	ID of the texture to filter
	 * @param	flipTextures	array of 2 other textures to ping-pong the effect between
	 * @param	size	the size of the textures provided (must all be the same)
	 * @param	amount	the intensity (number of passes to perform) */
	void applyGaussianBlur(const GLuint & desiredTexture, const GLuint * flipTextures, const vec2 & size, const int & amount);
	/** applyGaussianBlur_Alpha to the alpha channel of the desired texture.
	 * @note	requires two other textures to flip between to apply the filter
	 * @param	desiredTexture	ID of the texture to filter
	 * @param	flipTextures	array of 2 other textures to ping-pong the effect between
	 * @param	size	the size of the textures provided (must all be the same)
	 * @param	amount	the intensity (number of passes to perform) */
	void applyGaussianBlur_Alpha(const GLuint & desiredTexture, const GLuint * flipTextures, const vec2 & size, const int & amount);
	//void applyCubeFilter(const GLuint &sourceTexture, const GLuint & destinationTexture, const float &size);


private:
	// Private Methods
	/** Initializes the cubemap filter. */
	void initializeCubeFilter();
	/** Initializes the Gaussian blur filter. */
	void initializeGausianBlur();

	
	// Private Attributes
	bool m_Initialized;
	EnginePackage *m_enginePackage;
	Shared_Asset_Primitive m_shapeQuad;
	void* m_observer;
	Shared_Asset_Shader m_shaderGB, m_shaderGB_A, m_shaderCF;
	GLuint m_vao_Quad, m_fbo_GB;
};

#endif // VISUALFX