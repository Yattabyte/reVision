#pragma once
#ifndef GEOMETRY_BUFFER
#define GEOMETRY_BUFFER
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Utilities\GL\FrameBuffer.h"
#include "Utilities\GL\MappedBuffer.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"

class VisualFX;
class EnginePackage;


/**
 * A specialized framebuffer that accumulates surface attributes of all rendered objects for a single frame.
 * @brief	Objects render into it, storing their albedo, normal, specular, roughness, height, occlusion, and depth.
 **/
class DT_ENGINE_API Geometry_FBO : public FrameBuffer
{
public:
	// (de)Constructors
	/** Destroy the geometryFBO. */
	~Geometry_FBO();
	/** Construct the geometryFBO. */
	Geometry_FBO();

	
	// Public Methods
	/** Initialize the framebuffer.
	 * @param	enginePackage	the engine package
	 * @param	visualFX		reference to the post-processing utility class */
	void initialize(EnginePackage * enginePackage, VisualFX * visualFX);
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
	bool m_vaoLoaded;
	EnginePackage * m_enginePackage;
	VisualFX *m_visualFX;
	Shared_Asset_Shader m_shaderSSAO;	
	Shared_Asset_Primitive m_shapeQuad;
	MappedBuffer m_quadIndirectBuffer;
};

#endif // GEOMETRY_BUFFER