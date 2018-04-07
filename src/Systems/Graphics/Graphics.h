#pragma once
#ifndef SYSTEM_GRAPHICS
#define SYSTEM_GRAPHICS
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define MAX_KERNEL_SIZE 128 // Don't manipulate this, set the usable to a different value < than this

#include "Systems\System_Interface.h"
#include "Systems\Graphics\Resources\Frame Buffers\Geometry_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Lighting_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Shadow_FBO.h"
#include "Systems\Graphics\Resources\Frame Buffers\Reflection_FBO.h"
#include "Systems\Graphics\Resources\Geometry Techniques\Model_Techniques.h"
#include "Systems\Graphics\Resources\Lighting Techniques\Lighting_Technique.h"
#include "Systems\Graphics\FX Techniques\FX_Technique.h"
#include "Systems\Graphics\Resources\VisualFX.h"
#include "Systems\World\Visibility_Token.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\VectorBuffer.h"
#include <vector>

class EnginePackage;
class Camera;


struct Geometry_Struct {
	int useBones = 0;  // no padding here;
	GLuint materialID = 0; vec2 padding1; // for some reason padding here
	mat4 mMatrix = mat4(1.0f);
	mat4 transforms[NUM_MAX_BONES];
};

struct Reflection_Struct {
	mat4 mMatrix = mat4(1.0f);
	vec4 BoxCamPos = vec4(0.0f);
	float Radius = 1.0f;
	int CubeSpot = 0;
	vec2 padding;
};


/**
 * An engine system responsible for rendering. Creates Geometry_FBO, Lighting_FBO, Shadow_FBO, and VisualFX
 * @note	performs physically based rendering techniques.
 **/
class DT_ENGINE_API System_Graphics : public System
{
public: 
	// (de)Constructors
	/** Destroy the rendering system. */
	~System_Graphics();
	/** Construct the rendering system. */
	System_Graphics();
	

	// Interface Implementations
	virtual void initialize(EnginePackage * enginePackage);
	virtual void update(const float & deltaTime);
	virtual void updateThreaded(const float & deltaTime) {};


	// Public Methods
	/** Enable or Disable screen-space ambient occlusion.
	 * @param	ssao		true or false */
	void setSSAO(const bool & ssao);
	/** Change the number of samples used in screen-space ambient occlusion. 
	 * @param	samples		the number of samples to use */
	void setSSAOSamples(const int & samples);
	/** Change the strength of the screen-space ambient occlusion. 
	 * @param	strength	the strength to use */
	void setSSAOStrength(const int & strength);
	/** Change the radius of the screen-space ambient occlusion.
	 * @param	radius		the radius to use*/
	void setSSAORadius(const float & radius);
	

public:
	// Public Attributes
	// Frame Buffers
	Geometry_FBO m_geometryFBO;
	Lighting_FBO m_lightingFBO;
	Shadow_FBO m_shadowFBO;
	Reflection_FBO m_reflectionFBO;
	// Storage Buffers
	VectorBuffer<Geometry_Struct> m_geometrySSBO;
	VectorBuffer<Reflection_Struct> m_reflectionSSBO;
	
private:
	// Private Methods
	/** Regenerate the noise kernel. */
	void generateKernal();


	// Private Attributes
	ivec2 m_renderSize;
	VisualFX m_visualFX;
	StaticBuffer m_userBuffer;

	// Rendering Techniques
	vector<Geometry_Technique*> m_geometryTechs;
	vector<Lighting_Technique*> m_lightingTechs;
	vector<FX_Technique*> m_fxTechs;
};

#endif // SYSTEM_GRAPHICS