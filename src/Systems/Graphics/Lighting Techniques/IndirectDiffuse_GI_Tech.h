#pragma once
#ifndef INDIRECT_LIGHTING_PBR
#define INDIRECT_LIGHTING_PBR
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define GI_LIGHT_BOUNCE_COUNT 2 // Light bounces used
#define GI_TEXTURE_COUNT 4 // 3D textures used

#include "Systems\Graphics\Lighting Techniques\Lighting_Technique.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"

class EnginePackage;
class Geometry_Buffer;
class Lighting_Buffer;
class Shadow_Buffer;


/**
 * A lighting technique that calculates indirect diffuse lighting contribution for directional, point, and spot light types, using PBR techniques.
 **/
class DT_ENGINE_API IndirectDiffuse_GI_Tech : public Lighting_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~IndirectDiffuse_GI_Tech();
	/** Constructor. */
	IndirectDiffuse_GI_Tech(EnginePackage * enginePackage, Geometry_Buffer * gBuffer, Lighting_Buffer * lBuffer, Shadow_Buffer *sBuffer);


	// Interface Implementations.
	virtual void updateLighting(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	


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


	// Private Methods
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

	// Private Attributes
	Geometry_Buffer * m_gBuffer;
	Lighting_Buffer * m_lBuffer;
	Shadow_Buffer * m_sBuffer;
	EnginePackage * m_enginePackage;
	Shared_Asset_Shader m_shaderDirectional_Bounce, m_shaderPoint_Bounce, m_shaderSpot_Bounce, m_shaderGISecondBounce, m_shaderGIReconstruct;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO, m_bounceVAO;
	void* m_QuadObserver;
	GLuint m_fbo[GI_LIGHT_BOUNCE_COUNT]; // 1 fbo per light bounce
	GLuint m_textures[GI_LIGHT_BOUNCE_COUNT][GI_TEXTURE_COUNT]; // 4 textures per light bounce
	GLuint m_noise32;
	float m_nearPlane;
	float m_farPlane;
	GI_Attribs_Buffer m_attribBuffer;
	GLuint m_attribSSBO;
};

#endif // INDIRECT_LIGHTING_PBR