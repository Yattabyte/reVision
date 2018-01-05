/*
	Light_Directional_Component

	- A lighting technique that micicks the sun
*/

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

using namespace glm;

struct LightBuffer
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

	LightBuffer() {
		lightV = mat4(1.0f);
		LightColor = vec3(1.0f);
		LightDirection = vec3(0, -1, 0);
		ShadowSize = 0;
		LightIntensity = 0;
		CascadeIndex = 0;
		Use_Shadows = 0;
	}
};

class System_Shadowmap;
class Light_Directional_Creator;
class DT_ENGINE_API Light_Directional_Component : protected Lighting_Component
{
public:
	/*************
	----Common----
	*************/

	// Logic for interpreting receiving messages
	virtual void ReceiveMessage(const ECSmessage &message);


	/**********************************
	----Light_Directional Functions----
	**********************************/

	// Direct lighting pass
	virtual void directPass(const int &vertex_count);
	// Indirect lighting pass
	virtual void indirectPass(const int &vertex_count);
	// Shadow lighting pass
	virtual void shadowPass(const Visibility_Token &vis_token) const;
	// Returns whether or not this light is visible
	virtual bool IsVisible(const mat4 & PVMatrix);
	// Sends current data to the GPU
	void Update();


protected:
	~Light_Directional_Component();
	Light_Directional_Component(const ECShandle &id, const ECShandle &pid, Engine_Package *enginePackage);
	GLuint m_uboID;
	LightBuffer m_uboData;
	float m_cascadeEnd[5];
	System_Shadowmap *m_Shadowmapper;
	friend class Light_Directional_Creator;
};

class DT_ENGINE_API Light_Directional_Creator : public ComponentCreator
{
public:
	Light_Directional_Creator(ECSmessanger *ecsMessanger) : ComponentCreator(ecsMessanger) {}
	virtual Component* Create(const ECShandle &id, const ECShandle &pid, Engine_Package *enginePackage) {
		return new Light_Directional_Component(id, pid, enginePackage);
	}
};

#endif // LIGHT_DIRECTIONAL_COMPONENT