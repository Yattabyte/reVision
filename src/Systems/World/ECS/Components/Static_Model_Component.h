#pragma once
#ifndef STATIC_MODEL_COMPONENT
#define STATIC_MODEL_COMPONENT
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ

#include "Systems\World\ECS\Components\Geometry_Component.h"
#include "Assets\Asset_Model.h"
#include "Utilities\Transform.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"

using namespace glm;
class Static_Model_Creator;


/**
 * A renderable model component that supports animation.
 **/
class DT_ENGINE_API Static_Model_Component : protected Geometry_Component
{
public:
	// Interface implementations
	virtual const char * getName() const { return "Static_Model"; }
	virtual bool isLoaded() const;
	virtual bool isVisible(const float & radius, const vec3 & eyePosition) const;
	virtual bool containsPoint(const vec3 & point) const;
	virtual const ivec2 getDrawInfo() const;
	virtual const unsigned int getMeshSize() const;


	// Public Methods
	/** Retrieve the buffer index for this object.
	 * @return	the buffer index */
	const unsigned int getBufferIndex() const;


protected:
	// (de)Constructors
	/** Destroys a static model component. */
	~Static_Model_Component();
	/** Constructors an animated model component. */
	Static_Model_Component(EnginePackage *enginePackage);


	// Protected functions
	/** Update this component's bounding sphere */
	void updateBSphere();
	

	// Protected Attributes
	bool m_vaoLoaded;
	unsigned int m_uboIndex;
	float m_bsphereRadius;
	vec3 m_bspherePos;
	VB_Ptr * m_uboBuffer;
	GLuint m_skin;
	Shared_Asset_Model m_model;
	Transform m_transform;
	EnginePackage *m_enginePackage;
	friend class Static_Model_Creator;


private:
	// Private Functions
	/** Set the model directory and load the file. 
	 * @param	directory	the model directory */
	void setModelDirectory(const string & directory);
	/** Set the model skin index to use.
	 * @param	index		the index to use */
	void setSkin(const unsigned int & index);
	/** Set the transformation for this model.
	 * @param	transform	the transform to use */
	void setTransform(const Transform & transform);
};

class DT_ENGINE_API Static_Model_Creator : public ComponentCreator
{
public:
	Static_Model_Creator() : ComponentCreator() {}
	virtual Component* create(EnginePackage *enginePackage) {
		return new Static_Model_Component(enginePackage);
	}
};

#endif // STATIC_MODEL_COMPONENT