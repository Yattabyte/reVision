#pragma once
#ifndef GEOMETRY_BUFFER_H
#define GEOMETRY_BUFFER_H

#include "Utilities\GL\FrameBuffer.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"


class VisualFX;
class Engine;

/**
 * A specialized framebuffer that accumulates surface attributes of all rendered objects for a single frame.
 * @brief	Objects render into it, storing their albedo, normal, specular, roughness, height, occlusion, and depth.
 **/
class Geometry_FBO : public FrameBuffer
{
public:
	// (de)Constructors
	/** Destroy the geometryFBO. */
	~Geometry_FBO();
	/** Construct the geometryFBO. */
	Geometry_FBO();

	
	// Public Methods
	/** Initialize the framebuffer.
	 * @param	engine	the engine pointer
	 * @param	visualFX		reference to the post-processing utility class */
	void initialize(Engine * engine, VisualFX * visualFX);
	void initialize_noise();
	/** Binds and clears out all the render-targets in this framebuffer. */
	virtual void clear();
	/** Binds the framebuffer and its render-targets for writing. */
	virtual void bindForWriting();
	/** Binds the framebuffer and its render-targets for reading. */
	virtual void bindForReading();
	/** Change the size of the framebuffer object. 
	 * @param	size		the new size of the framebuffer */
	virtual void resize(const ivec2 & isize);
	/** Resets the framebuffer and re-attaches all its render-targets.*/
	void end();
	/** Generate ambient occlusion for the frame. */
	void applyAO();
	/** Bind the depth buffer for writing. */
	void bindDepthWriting();
	/** Bind the depth buffer for reading, to the target specified. */
	void bindDepthReading(const unsigned int & textureUnit);


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
	GLuint m_noiseID, m_quadVAO;
	bool m_quadVAOLoaded;
	Engine * m_engine;
	VisualFX *m_visualFX;
	Shared_Asset_Shader m_shaderSSAO;	
	Shared_Asset_Primitive m_shapeQuad;
	StaticBuffer m_quadIndirectBuffer;
};

#endif // GEOMETRY_BUFFER_H