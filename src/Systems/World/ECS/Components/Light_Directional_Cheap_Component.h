#pragma once
#ifndef LIGHT_DIRECTIONAL_CHEAP_COMPONENT
#define LIGHT_DIRECTIONAL_CHEAP_COMPONENT
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
#include "Utilities\Transform.h"


using namespace glm;
class Light_Directional_Cheap_Creator;
class EnginePackage;

/**
 * A renderable light component that mimics the sun.
 * A cheap variation, no shadows or GI.
 **/
class DT_ENGINE_API Light_Directional_Cheap_Component : protected Lighting_Component
{
public:
	// Interface implementations
	virtual const char * getName() const { return "Light_Directional_Cheap"; }
	virtual bool isVisible(const float & radius, const vec3 & eyePosition) const;
	virtual float getImportance(const vec3 &position) const;
	virtual void occlusionPass() {}
	virtual void shadowPass() {}
	virtual	void update() {}


protected:
	// (de)Constructors
	/** Destroys a cheap directional light component. */
	~Light_Directional_Cheap_Component();
	/** Constructs a cheap directional light component. */
	Light_Directional_Cheap_Component(EnginePackage *enginePackage);


	// Protected Attributes
	friend class Light_Directional_Cheap_Creator;


private:
	// Private Functions
	/** Set the light color to use.
	* @param	color		the color to use */
	void setColor(const vec3 & color);
	/** Set the light intensity to use.
	* @param	intensity	the intensity to use */
	void setIntensity(const float & intensity);
	/** Set the transformation for this component.
	* @param	transform	the transform to use */
	void setTransform(const Transform & transform);
};

class DT_ENGINE_API Light_Directional_Cheap_Creator : public ComponentCreator
{
public:
	Light_Directional_Cheap_Creator() : ComponentCreator() {}
	virtual Component* create(EnginePackage *enginePackage) {
		return new Light_Directional_Cheap_Component(enginePackage);
	}
};

#endif // LIGHT_DIRECTIONAL_CHEAP_COMPONENT