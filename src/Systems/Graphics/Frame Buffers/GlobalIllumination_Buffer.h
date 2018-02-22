#pragma once
#ifndef GLOBAL_ILLUMINATION_BUFFER
#define GLOBAL_ILLUMINATION_BUFFER
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLEW_STATIC
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define GI_LIGHT_BOUNCE_COUNT 2 // Light bounces used
#define GI_TEXTURE_COUNT 4 // 3D textures used

#include "GL\glew.h"
#include "glm\glm.hpp"

using namespace glm;
class EnginePackage;


/**
 * A specialized framebuffer used for combining previous rendering phases and applying HDR+Bloom.
 **/
class DT_ENGINE_API GlobalIllumination_Buffer
{
public:
	// (de)Constructors
	/** Destroy the HDRBuffer. */
	~GlobalIllumination_Buffer();
	/** Construct the HDRBuffer. */
	GlobalIllumination_Buffer();


	// Public Methods
	/** Initialize the framebuffer.
	 * @param	volumeSize		the volume size of the framebuffer 
	 * @param	enginePackage	the engine package */
	void initialize(const unsigned int & volumeSize, EnginePackage * enginePackage);
	/** Binds and clears out all the render-targets in this framebuffer. */
	void clear();
	/** Binds the framebuffer and its render-targets for writing.
	 * @param	bounceSpot		which bounce we are performing */
	void bindForWriting(const GLuint & bounceSpot);
	/** Binds the framebuffer and its render-targets for reading.
	 * @param	bounceSpot		which bounce we are performing
	 * @param	textureUnit		which texture unit we are going to start with (minimum GL_TEXTURE0) */
	void bindForReading(const GLuint & bounceSpot, const GLuint textureUnit);
	/** Bind the noise texture
	 * @param	textureUnit		the texture unit to bind the noise texture */
	void bindNoise(const GLuint textureUnit);
	/** Recalculate underlying data */
	void updateData();


private:
	/** Nested buffer object struct for sending data to GPU */
	struct GI_Attribs_Buffer
	{
		vec4 BBox_Max;
		vec4 BBox_Min;
		int samples;
		int num_lights;
		int resolution;
		float spread;
		float R_wcs;
		float factor;
		GI_Attribs_Buffer() {
			BBox_Max = vec4(1);
			BBox_Min = vec4(-1);
			samples = 16;
			num_lights = 1;
			resolution = 16;
			spread = 1.0f;
			R_wcs = 1.0f;
			factor = 1.0f;
		};
		GI_Attribs_Buffer(const int &res, const float &rad, const float &wrld, const float &blend, const int &smp) : GI_Attribs_Buffer()
		{
			resolution = res;
			spread = rad;
			R_wcs = wrld;
			factor = blend;
			samples = smp;
		}
		void updateBBOX(const vec3 &max, const vec3&min, const vec3& pos) {
			BBox_Max = vec4(max + pos, 1.0f);
			BBox_Min = vec4(min + pos, 1.0f);
		}
	};

	// Private Attributes
	GLuint m_fbo[GI_LIGHT_BOUNCE_COUNT]; // 1 fbo per light bounce
	GLuint m_textures[GI_LIGHT_BOUNCE_COUNT][GI_TEXTURE_COUNT]; // 4 textures per light bounce
	GLuint m_noise32;
	float m_nearPlane;
	float m_farPlane;
	bool m_Initialized;
	GI_Attribs_Buffer m_attribBuffer;
	GLuint m_attribSSBO;
	EnginePackage *m_enginePackage;
};

#endif // GLOBAL_ILLUMINATION_BUFFER