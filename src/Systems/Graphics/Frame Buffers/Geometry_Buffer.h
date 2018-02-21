#pragma once
#ifndef GEOMETRY_BUFFER
#define GEOMETRY_BUFFER
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
class VisualFX;


/**
 * A specialized framebuffer that accumulates surface attributes of all rendered objects for a single frame.
 * @brief	Objects render into it, storing their albedo, normal, specular, roughness, height, occlusion, and depth.
 **/
class DT_ENGINE_API Geometry_Buffer
{
public:
	// (de)Constructors
	/** Destroy the gBuffer. */
	~Geometry_Buffer();
	/** Construct the gBuffer. */
	Geometry_Buffer();

	
	// Public Methods
	/** Initialize the framebuffer.
	 * @param	size		the size of the framebuffer
	 * @param	visualFX	reference to the post-processing utility class */
	void initialize(const vec2 & size, VisualFX * visualFX);
	/** Binds and clears out all the render-targets in this framebuffer. */
	void clear();
	/** Binds the framebuffer and its render-targets for writing. */
	void bindForWriting();
	/** Binds the framebuffer and its render-targets for reading. */
	void bindForReading();
	/** Resets the framebuffer and re-attaches all its render-targets.*/
	void end();
	/** Change the size of the framebuffer object. 
	 * @param	size		the new size of the framebuffer */
	void resize(const vec2 & size);
	/** Generate ambient occlusion for the frame. */
	void applyAO();


	// Public attributes
	/** Enumeration for indexing into m_textures. */
	static const enum GBUFFER_TEXTURE_TYPE {
		GBUFFER_TEXTURE_TYPE_IMAGE,
		GBUFFER_TEXTURE_TYPE_VIEWNORMAL,
		GBUFFER_TEXTURE_TYPE_SPECULAR,
		GBUFFER_NUM_TEXTURES
	};
	GLuint m_textures[GBUFFER_NUM_TEXTURES], m_texturesGB[2], m_depth_stencil;


private:
	// Private attributes
	GLuint m_fbo, m_noiseID;
	VisualFX *m_visualFX;
	Shared_Asset_Shader m_shaderSSAO;	
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_vao_Quad;
	vec2 m_renderSize;
	bool m_Initialized;
	void *m_observer;
};

#endif // GEOMETRY_BUFFER