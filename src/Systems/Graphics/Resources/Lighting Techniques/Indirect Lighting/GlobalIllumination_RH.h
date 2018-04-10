#pragma once
#ifndef GLOBALILLUMINATION_RH
#define GLOBALILLUMINATION_RH
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

#include "Systems\Graphics\Resources\Lighting Techniques\Lighting_Technique.h"
#include "Systems\World\Camera.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"

class EnginePackage;
class Geometry_FBO;
class Lighting_FBO;
class Shadow_FBO;


 /**
 * Performs primary and secondary light bounces, using the radiance hints technique.
 * Responsible for indirect diffuse lighting.
 * Supports physically based shaders.
 * Supports directional, point, and spot lights.
 **/
class DT_ENGINE_API GlobalIllumination_RH : public Lighting_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~GlobalIllumination_RH();
	/** Constructor. */
	GlobalIllumination_RH(EnginePackage * enginePackage, Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, Shadow_FBO *shadowFBO);


	// Interface Implementations.
	virtual void updateData(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	


private:
	/** Nested buffer object struct for sending data to GPU */
	struct GI_Attribs_Buffer
	{
		vec4 BBox_Max;
		vec4 BBox_Min;
		int samples;
		int resolution;
		float spread;
		float R_wcs;
		float factor;
		GI_Attribs_Buffer() {
			BBox_Max = vec4(1);
			BBox_Min = vec4(-1);
			samples = 16;
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
	* @param	textureUnit		which texture unit we are going to start with (minimum 0) */
	void bindForReading(const GLuint & bounceSpot, const unsigned int & textureUnit);
	/** Bind the noise texture
	* @param	textureUnit		the texture unit to bind the noise texture */
	void bindNoise(const GLuint textureUnit);
	/** Recalculate underlying data */
	void update();


	// Private Attributes
	Geometry_FBO * m_geometryFBO;
	Lighting_FBO * m_lightingFBO;
	Shadow_FBO * m_shadowFBO;
	EnginePackage * m_enginePackage;
	Shared_Asset_Shader m_shaderDirectional_Bounce, m_shaderPoint_Bounce, m_shaderSpot_Bounce, m_shaderGISecondBounce, m_shaderGIReconstruct;
	Shared_Asset_Primitive m_shapeQuad;
	bool m_vaoLoaded;
	GLuint m_quadVAO, m_bounceVAO;
	GLuint m_fbo[GI_LIGHT_BOUNCE_COUNT]; // 1 fbo per light bounce
	GLuint m_textures[GI_LIGHT_BOUNCE_COUNT][GI_TEXTURE_COUNT]; // 4 textures per light bounce
	GLuint m_noise32;
	float m_nearPlane;
	float m_farPlane;
	GI_Attribs_Buffer m_attribBuffer;
	GLuint m_attribSSBO;
	Camera m_camera;
	void *m_bufferPtr;
	StaticBuffer m_pointsIndirectBuffer, m_pointsSecondIndirectBuffer, m_quadIndirectBuffer;
};

#endif // GLOBALILLUMINATION_RH