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
class VisualFX;


/**
 * A specialized framebuffer that accumulates lighting information for a single frame.
 * Supports bloom, accumulates over-brightened lights in a second render-target.
 **/
class DT_ENGINE_API Lighting_Buffer
{
public:
	// (de)Constructors
	/** Destroy the lighting buffer. */
	~Lighting_Buffer();
	/** Destroy the lighting buffer. */
	Lighting_Buffer();


	// Methods
	/** Initialize the framebuffer.
	 * @param	size	the size of the framebuffer
	 * @param	visualFX	reference to the post-processing utility class
	 * @param	bloomStrength	intensity / number of passes for the bloom effect
	 * @param	depthStencil	reference to the depthStencil texture from the gBuffer */
	void initialize(const vec2 & size, VisualFX * visualFX, const int & bloomStrength, const GLuint & depthStencil);
	/** Binds and clears out all the render-targets in this framebuffer. */
	void clear();
	/** Binds the framebuffer and its render-targets for writing. */
	void bindForWriting();
	/** Binds the framebuffer and its render-targets for reading. */
	void bindForReading();
	/** Change the size of the framebuffer object. 
	 * @param	size	the new size of the framebuffer */
	void resize(const vec2 & size);
	/** Change the strength of the bloom effect.
	 * @param	strength	the new strength of the bloom effect */
	void setBloomStrength(const int &strength);
	/** Apply blur filter to bloom attachment, finishing the bloom effect. */
	void applyBloom();
	

private:
	/** Nested enumeration for indexing into m_textures. */
	static const enum LBUFFER_TEXTURE_TYPE {
		LBUFFER_TEXTURE_TYPE_SCENE,
		LBUFFER_TEXTURE_TYPE_OVERBRIGHT,
		LBUFFER_NUM_TEXTURES
	};


	// Private Attributes
	GLuint m_fbo, m_textures[LBUFFER_NUM_TEXTURES], m_texturesGB[2];
	vec2 m_renderSize;
	VisualFX *m_visualFX;
	GLuint m_depth_stencil; // Donated by the geometry buffer
	int m_bloomStrength;
	bool m_Initialized;
};

#endif // LIGHTING_BUFFER