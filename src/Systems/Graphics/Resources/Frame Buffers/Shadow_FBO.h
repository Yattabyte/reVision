#pragma once
#ifndef SYSTEM_SHADOWMAP
#define SYSTEM_SHADOWMAP
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define SHADOW_REGULAR 0 
#define SHADOW_LARGE 1
#define SHADOW_MAX 2

#include "GL\glew.h"
#include "glm\glm.hpp"
#include <deque>

using namespace std;
using namespace glm;
class EnginePackage;


/**
 * An engine system that manages the creation and storage of shadowmap's for lights
 **/
class DT_ENGINE_API Shadow_FBO
{
public:
	// (de)Constructors
	/** Destroy the shadow buffer. */
	~Shadow_FBO();
	/** Construct the shadow buffer. */
	Shadow_FBO();
	

	// Public Methods
	/** Initialize the framebuffer.
	 * @param	enginePackage		the engine package */
	virtual void initialize(EnginePackage * enginePackage);
	/** Register a new shadow caster into the system.
	 * @param	shadow_type			the shadow type this caster needs (regular/large)
	 * @param	array_spot			reference to be updated with index into shadowmap texture */
	void registerShadowCaster(const int & shadow_type, int & array_spot);
	/** Removes the registration of a particular shadow caster from the system.
	 * @param	shadow_type			the shadow type of this caster
	 * @param	array_spot			index into the shadowmap texture */
	void unregisterShadowCaster(const int & shadow_type, int & array_spot);
	/** Binds the framebuffer and its render-targets for writing.
	 * @param	shadow_type			the index to write into */
	void bindForWriting(const int & shadow_type);
	/** Binds the framebuffer and its render-targets for reading. 
	 * @param	shadow_type			the type of shadow to be written
	 * @param	ShaderTextureUnit	the texture unit to bind the texture to */
	void bindForReading(const int & shadow_type, const GLuint & ShaderTextureUnit);	
	/** Binds the framebuffer and its render-targets for reading, specifically for the global illumination pass. 
	 * @param	shadow_type			the type of shadow to be written
	 * @param	ShaderTextureUnit	the texture unit to bind the texture to*/
	void BindForReading_GI(const int &ShadowSpot, const GLuint &ShaderTextureUnit);
	/** Binds and clears out the render-targets in this framebuffer.
	 * @param	shadow_type			the type of shadow to be written
	 * @param	layer				index of the shadow to clear */
	void clearShadow(const int & shadow_type, const int & layer);
	/** Change the size of the framebuffer for a particular shadow type.
	 * @param	shadow_type			the type of shadow
	 * @param	size				the new size to use */
	void setSize(const unsigned int & shadow_type, const float & size);
	/** Retrieve the size of the framebuffer for a particular shadow type.
	 * @param	shadow_type			the type of shadow
	 * @return						size of the shadow texture */
	vec2 getSize(const unsigned int & shadow_type);


private:
	// Private Attributes
	bool				m_Initialized;
	vec2				m_size[SHADOW_MAX];
	GLuint				m_shadow_fbo[SHADOW_MAX];
	GLuint				m_shadow_depth[SHADOW_MAX];
	GLuint				m_shadow_worldnormal[SHADOW_MAX];
	GLuint				m_shadow_radiantflux[SHADOW_MAX];
	unsigned int		m_shadow_count[SHADOW_MAX];
	deque<unsigned int>	m_freed_shadow_spots[SHADOW_MAX];
	EnginePackage	   *m_enginePackage;
};

#endif // SYSTEM_SHADOWMAP