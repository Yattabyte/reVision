#pragma once
#ifndef VISUALFX_H
#define VISUALFX_H

#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"
#include "GL\glew.h"
#include "glm\glm.hpp"


class Engine;

/** A utility class that applies graphical effects/filters to images, such as Gaussian blur.*/
class VisualFX {
public:
	// (de)Constructors
	/** Destructor. */
	~VisualFX();
	/** Constructor. */
	VisualFX() = default;

	
	// Public Methods
	/** Initialize this utility and all its filters.
	@param	engine	the engine pointer */
	void initialize(Engine * engine);	
	/** Apply a Gaussian blur filter to the desired texture.
	@note					requires two other textures to flip between to apply the filter
	@param	desiredTexture	ID of the texture to filter
	@param	flipTextures	array of 2 other textures to ping-pong the effect between
	@param	size			the size of the textures provided (must all be the same)
	@param	amount			the intensity (number of passes to perform) */
	void applyGaussianBlur(const GLuint & desiredTexture, const GLuint * flipTextures, const glm::vec2 & size, const int & amount);
	/** applyGaussianBlur_Alpha to the alpha channel of the desired texture.
	@note					requires two other textures to flip between to apply the filter
	@param	desiredTexture	ID of the texture to filter
	@param	flipTextures	array of 2 other textures to ping-pong the effect between
	@param	size			the size of the textures provided (must all be the same)
	@param	amount			the intensity (number of passes to perform) */
	void applyGaussianBlur_Alpha(const GLuint & desiredTexture, const GLuint * flipTextures, const glm::vec2 & size, const int & amount);
	//void applyCubeFilter(const GLuint &sourceTexture, const GLuint & destinationTexture, const float &size);


private:
	// Private Methods
	/** Initializes the cubemap filter. */
	void initializeCubeFilter();
	/** Initializes the Gaussian blur filter. */
	void initializeGausianBlur();

	
	// Private Attributes
	Engine * m_engine = nullptr;
	bool m_Initialized = false;
	Shared_Asset_Primitive m_shapeQuad;
	Shared_Asset_Shader m_shaderGB, m_shaderGB_A, m_shaderCF;
	GLuint m_quadVAO = 0, m_fbo_GB = 0;
	bool m_quadVAOLoaded = false;
	StaticBuffer m_quadIndirectBuffer;
};

#endif // VISUALFX_H