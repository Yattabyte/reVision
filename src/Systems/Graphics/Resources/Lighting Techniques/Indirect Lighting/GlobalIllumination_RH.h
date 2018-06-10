#pragma once
#ifndef GLOBALILLUMINATION_RH_H
#define GLOBALILLUMINATION_RH_H
#define GI_LIGHT_BOUNCE_COUNT 2 // Light bounces used
#define GI_TEXTURE_COUNT 4 // 3D textures used

#include "Systems\Graphics\Resources\Lighting Techniques\Lighting_Technique.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Base Types\Light_Tech.h"
#include "Systems\World\Camera.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Utilities\GL\StaticBuffer.h"

class EnginePackage;
class Geometry_FBO;
class Lighting_FBO;


 /**
 * Performs primary and secondary light bounces, using the radiance hints technique.
 * Responsible for indirect diffuse lighting.
 * Supports physically based shaders.
 * Supports directional, point, and spot lights.
 **/
class GlobalIllumination_RH : public Lighting_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~GlobalIllumination_RH();
	/** Constructor. */
	GlobalIllumination_RH(
		EnginePackage * enginePackage, 
		Geometry_FBO * geometryFBO, Lighting_FBO * lightingFBO, 
		vector<Light_Tech*> * baseTechs
	);


	// Interface Implementations.
	virtual void updateData(const Visibility_Token & vis_token);
	virtual void applyPrePass(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	


private:
	// Private Methods
	/** Binds the framebuffer and its render-targets for writing.
	 * @param	bounceSpot		which bounce we are performing */
	void bindForWriting(const GLuint & bounceSpot);
	/** Binds the framebuffer and its render-targets for reading.
	 * @param	bounceSpot		which bounce we are performing
	 * @param	textureUnit		which texture unit we are going to start with (minimum 0) */
	void bindForReading(const GLuint & bounceSpot, const unsigned int & textureUnit);
	/** Bind the noise texture.
	 * @param	textureUnit		the texture unit to bind the noise texture */
	void bindNoise(const GLuint textureUnit);


	// Private Attributes
	EnginePackage * m_enginePackage;
	vector<Light_Tech*> * m_baseTechs;
	// Shared FBO's
	Geometry_FBO * m_geometryFBO;
	Lighting_FBO * m_lightingFBO;
	Shared_Asset_Shader m_shaderGISecondBounce, m_shaderGIReconstruct;
	Shared_Asset_Primitive m_shapeQuad;
	bool m_vaoLoaded;
	GLuint m_quadVAO;
	GLuint m_fbo[GI_LIGHT_BOUNCE_COUNT]; // 1 fbo per light bounce
	GLuint m_textures[GI_LIGHT_BOUNCE_COUNT][GI_TEXTURE_COUNT]; // 4 textures per light bounce
	GLuint m_noise32;
	float m_nearPlane;
	float m_farPlane;
	ivec2 m_renderSize;
	GLuint m_resolution;
	Camera m_camera;
	StaticBuffer m_attribBuffer, m_IndirectSecondLayersBuffer, m_quadIndirectBuffer;
};

#endif // GLOBALILLUMINATION_RH_H