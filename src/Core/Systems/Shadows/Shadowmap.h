/*
	Shadowmap

	- A system that manages the creation and storage of shadowmaps for lights
*/



#pragma once
#ifndef SYSTEM_SHADOWMAP
#define SYSTEM_SHADOWMAP
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define SHADOW_REGULAR 0 
#define SHADOW_LARGE 1
#define SHADOW_MAX 2

#include "Systems\System_Interface.h"
#include "GL\glew.h"
#include "glm\glm.hpp"
#include <deque>

using namespace std;
using namespace glm;

class Callback_Container;
class Engine_Package;
class DT_ENGINE_API System_Shadowmap : public System
{
public: 
	~System_Shadowmap();
	System_Shadowmap();
	void Initialize(Engine_Package *enginePackage);

	// Recalculate visibility
	void Update(const float &deltaTime);
	void Update_Threaded(const float &deltaTime);
	
	void RegisterShadowCaster(const int & shadow_type, int & array_spot);
	void UnRegisterShadowCaster(const int & shadow_type, int & array_spot);
	void BindForWriting(const int & ShadowSpot);
	void BindForReading(const int & ShadowSpot, const GLuint & ShaderTextureUnit);
	void Test(); 
	void ClearShadow(const int & ShadowSpot, const int & layer, const int &depth);
	void SetSize(const unsigned int &spot, const float &size);
	vec2 GetSize(const unsigned int &spot);
	void SetUpdateQuality(const float &quality);

private:
	vec2				m_size[SHADOW_MAX];
	GLuint				m_shadow_fbo[SHADOW_MAX];
	GLuint				m_shadow_depth[SHADOW_MAX];
	GLuint				m_shadow_worldpos[SHADOW_MAX];
	GLuint				m_shadow_worldnormal[SHADOW_MAX];
	GLuint				m_shadow_radiantflux[SHADOW_MAX];
	unsigned int		m_shadow_count[SHADOW_MAX];
	deque<unsigned int>	m_freed_shadow_spots[SHADOW_MAX];
	Callback_Container *m_largeChangeCallback, *m_RegularChangeCallback;
};

#endif // SYSTEM_SHADOWMAP