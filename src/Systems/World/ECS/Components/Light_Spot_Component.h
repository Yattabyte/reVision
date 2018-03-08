#pragma once
#ifndef LIGHT_SPOT_COMPONENT
#define LIGHT_SPOT_COMPONENT
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"
#include "Systems\World\ECS\Components\Lighting_Component.h"
#include "Systems\World\Camera.h"

using namespace glm;
class Shadow_Buffer;
class System_World;
class Light_Spot_Creator;
class EnginePackage;


/**
 * A renderable light component that mimics a flashlight.
 * Uses a single shadow map.
 **/
class DT_ENGINE_API Light_Spot_Component : protected Lighting_Component
{
public:
	// Interface Implementations
	virtual void receiveMessage(const ECSmessage &message);
	virtual void directPass(const int &vertex_count);
	virtual void indirectPass(const int &vertex_count);
	virtual void shadowPass();
	virtual bool isVisible(const mat4 & PMatrix, const mat4 &VMatrix);
	virtual float getImportance(const vec3 &position) const;


	// Public Methods
	/** Sends current data to the GPU. */
	void update();
	/** Retrieves the shadow spot for this light
	* @return	the shadow spot requested */
	GLuint getShadowSpot() const;


protected:
	/** Nested Buffer class.
	 * @brief	used for sending data to the gpu. */
	struct LightSpotBuffer
	{
		mat4 mMatrix;
		mat4 lightV;
		mat4 lightP;
		vec3 LightColor; float padding1;
		vec3 LightPosition; float padding2;
		vec3 LightDirection; float padding3;
		float ShadowSize;
		float LightIntensity;
		float LightRadius;
		float LightCutoff;
		int Shadow_Spot;
		int Use_Shadows;
		int LightStencil;

		LightSpotBuffer() {
			mMatrix = mat4(1.0f);
			lightV = mat4(1.0f);
			lightP = mat4(1.0f);
			LightColor = vec3(1.0f);
			LightPosition = vec3(0.0f);
			LightDirection = vec3(0, -1, 0);
			ShadowSize = 0;
			LightIntensity = 0;
			LightRadius = 0;
			LightCutoff = 0;
			Shadow_Spot = 0;
			Use_Shadows = 0;
			LightStencil = 0;
		}
	};


	// (de)Constructors
	/** Destroys a spot light component. */
	~Light_Spot_Component();
	/** Constructs a spot light component. */
	Light_Spot_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage);


	// Protected Attributes
	GLuint m_uboID;
	LightSpotBuffer m_uboData;
	EnginePackage *m_enginePackage;
	Shadow_Buffer *m_shadowMapper;
	System_World *m_world;
	quat m_orientation;
	float m_squaredRadius;
	Camera m_camera;
	friend class Light_Spot_Creator;
};

class DT_ENGINE_API Light_Spot_Creator : public ComponentCreator
{
public:
	Light_Spot_Creator(ECSmessenger *ecsMessenger) : ComponentCreator(ecsMessenger) {}
	virtual Component* Create(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage) {
		return new Light_Spot_Component(id, pid, enginePackage);
	}
};

#endif // LIGHT_SPOT_COMPONENT