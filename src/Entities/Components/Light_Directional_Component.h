#pragma once
#ifndef LIGHT_DIRECTIONAL_COMPONENT
#define LIGHT_DIRECTIONAL_COMPONENT
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else            
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define LIGHT_TYPE_DIRECTIONAL 0
#define NUM_CASCADES 4
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include "GLM\gtc\type_ptr.hpp"
#include "GL\glew.h"
#include "Entities\Components\Lighting_Component.h"
#include "Systems\World\Camera.h"

using namespace glm;
class Light_Directional_Creator;
class EnginePackage;


/**
 * A renderable light component that mimics the sun.
 * Uses cascaded shadowmaps.
 **/
class DT_ENGINE_API Light_Directional_Component : protected Lighting_Component
{
public:
	// Methods
	/** Sends current data to the GPU. */
	void update();
	/** Recalculates the shadowmap cascades. */
	void calculateCascades();


	// Interface implementation
	virtual void receiveMessage(const ECSmessage &message);
	virtual void directPass(const int &vertex_count);
	virtual void indirectPass(const int &vertex_count);
	virtual void shadowPass();
	virtual bool isVisible(const mat4 & PMatrix, const mat4 &VMatrix);
	virtual float getImportance(const vec3 &position);	


protected:
	/** Nested Buffer class.
	 * @brief	used for sending data to the gpu. */
	struct LightDirBuffer
	{
		mat4 lightV;
		vec3 LightColor; float padding1;
		vec3 LightDirection; float padding2;
		float ShadowSize;
		float LightIntensity;
		int CascadeIndex;
		int Use_Shadows;

		// These need to be padded to 16 bytes each, because of layout std140 rules for array elements
		ivec4 Shadow_Spot[NUM_CASCADES]; // first element used only
		vec4 CascadeEndClipSpace[NUM_CASCADES]; // first element used only
		mat4 lightP[NUM_CASCADES]; // these are good already

		LightDirBuffer() {
			lightV = mat4(1.0f);
			LightColor = vec3(1.0f);
			LightDirection = vec3(0, -1, 0);
			ShadowSize = 0;
			LightIntensity = 0;
			CascadeIndex = 0;
			Use_Shadows = 0;
		}
	};


	// (de)Constructors
	/** Destroys a directional light component. */
	~Light_Directional_Component();
	/** Constructs a directional light component. */
	Light_Directional_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage);


	// Attributes
	GLuint m_uboID;
	LightDirBuffer m_uboData;
	float m_cascadeEnd[5];
	EnginePackage *m_enginePackage;
	Camera m_camera;
	friend class Light_Directional_Creator;
};

class DT_ENGINE_API Light_Directional_Creator : public ComponentCreator
{
public:
	Light_Directional_Creator(ECSmessanger *ecsMessanger) : ComponentCreator(ecsMessanger) {}
	virtual Component* Create(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage) {
		return new Light_Directional_Component(id, pid, enginePackage);
	}
};

#endif // LIGHT_DIRECTIONAL_COMPONENT