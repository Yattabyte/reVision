#pragma once
#ifndef SKYBOX
#define SKYBOX
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\Graphics\Resources\Lighting Techniques\Lighting_Technique.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Cubemap.h"
#include "Utilities\GL\StaticBuffer.h"

class Lighting_FBO;


/**
 * A lighting technique that applies a screen space skybox effect.
 **/
class DT_ENGINE_API Skybox : public Lighting_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Skybox();
	/** Constructor. */
	Skybox(Lighting_FBO * lightingFBO);


	// Interface Implementations
	virtual void updateData(const Visibility_Token & vis_token);
	virtual void applyPrePass(const Visibility_Token & vis_token);
	virtual void applyLighting(const Visibility_Token & vis_token);	


private:
	// Private Attributes
	Lighting_FBO * m_lightingFBO;
	Shared_Asset_Shader	m_shaderSky;
	Shared_Asset_Cubemap m_textureSky;
	Shared_Asset_Primitive m_shapeQuad;
	GLuint m_quadVAO;
	bool m_vaoLoaded;
	StaticBuffer m_quadIndirectBuffer;
};

#endif // SKYBOX