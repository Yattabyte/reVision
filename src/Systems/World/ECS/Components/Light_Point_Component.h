#pragma once
#ifndef LIGHT_POINT_COMPONENT
#define LIGHT_POINT_COMPONENT
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
class Shadow_FBO;
class System_World;
class Light_Point_Creator;
class EnginePackage;


/**
 * A renderable light component that mimics a light-bulb.
 * Uses dual paraboloid shadow maps.
 **/
class DT_ENGINE_API Light_Point_Component : protected Lighting_Component
{
public:
	// Interface Implementations
	virtual void receiveMessage(const ECSmessage &message);
	virtual void directPass(const int &vertex_count);
	virtual void indirectPass();
	virtual void shadowPass();
	virtual bool isVisible(const mat4 & PMatrix, const mat4 &VMatrix);
	virtual float getImportance(const vec3 &position) const;
	

	/** Sends current data to the GPU. */
	void update();
	/** Retrieves the front or rear shadow spot.
	 * @param	front	set to true to retrieve the front, or false for the back
	 * @return			the shadow spot requested */
	GLuint getShadowSpot(const bool &front) const;


protected:
	/** Nested Buffer class.
	 * @brief	used for sending data to the gpu. */
	struct LightPointBuffer {
		mat4 mMatrix;
		mat4 lightV;
		vec3 LightColor; float padding1;
		vec3 LightPosition; float padding2;
		float p_far;
		float p_dir;
		float ShadowSize;
		float LightIntensity;
		float LightRadius;
		int Shadow_Spot1;
		int Shadow_Spot2;
		int Use_Shadows;
		int LightStencil;

		LightPointBuffer() {
			mMatrix = mat4(1.0f);
			lightV = mat4(1.0f);
			LightColor = vec3(1.0f);
			LightPosition = vec3(0.0f);
			p_far = 0;
			p_dir = 0;
			ShadowSize = 0;
			LightIntensity = 0;
			LightRadius = 0;
			Shadow_Spot1 = 0;
			Shadow_Spot2 = 0;
			Use_Shadows = 0;
			LightStencil = 0;
		}
	};


	// (de)Constructors
	/** Destroys a point light component. */
	~Light_Point_Component();
	/** Constructs a point light component. */
	Light_Point_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage);


	// Protected Attributes
	GLuint m_uboID;
	LightPointBuffer m_uboData;
	EnginePackage *m_enginePackage;
	Shadow_FBO *m_shadowMapper;
	System_World *m_world;
	float m_squaredRadius;
	Camera m_camera[2];
	friend class Light_Point_Creator;
};

class DT_ENGINE_API Light_Point_Creator : public ComponentCreator
{
public:
	Light_Point_Creator(ECSmessenger *ecsMessenger) : ComponentCreator(ecsMessenger) {}
	virtual Component* Create(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage) {
		return new Light_Point_Component(id, pid, enginePackage);
	}
};

#endif // LIGHT_POINT_COMPONENT