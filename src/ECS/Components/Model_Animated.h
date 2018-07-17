#pragma once
#ifndef ANIM_MODEL_COMPONENT_H
#define ANIM_MODEL_COMPONENT_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ

#include "ECS\Components\Geometry.h"
#include "Assets\Asset_Model.h"
#include "Utilities\Transform.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"


class Anim_Model_Creator;

/**
 * A renderable model component that supports animation.
 **/
class Model_Animated_C : protected Geometry_C
{
public:
	// Interface implementations
	virtual const char * getName() const { return "Anim_Model"; }
	virtual bool isLoaded() const;
	virtual bool isVisible(const float & radius, const glm::vec3 & eyePosition) const;
	virtual bool containsPoint(const glm::vec3 & point) const;
	virtual const glm::ivec2 getDrawInfo() const;
	virtual const unsigned int getMeshSize() const;


	// Public Methods
	/** Retrieve the buffer index for this object.
	 * @return	the buffer index */
	const unsigned int getBufferIndex() const;
	/** Ticks ahead the animation state by the amount of deltaTime. */
	void animate(const double &deltaTime);


protected:
	// (de)Constructors
	/** Destroys an animated model component. */
	~Model_Animated_C();
	/** Constructors an animated model component. */
	Model_Animated_C(Engine * engine, const std::string & filename = "", const unsigned int & skinIndex = 0, const unsigned int & animationIndex = 0, const Transform & transform = Transform());


	// Protected functions
	/** Update this component's bounding sphere */
	void updateBSphere();
	

	// Protected Attributes
	int m_animation;
	bool m_playAnim;
	float m_animTime, m_animStart;
	bool m_vaoLoaded;
	unsigned int m_uboIndex;
	float m_bsphereRadius;
	glm::vec3 m_bspherePos;
	VB_Ptr * m_uboBuffer;
	GLuint m_skin;
	Shared_Asset_Model m_model;
	Transform m_transform;
	std::vector<BoneTransform> m_transforms;
	Engine *m_engine;
	friend class Anim_Model_Creator;


private:
	// Private Functions
	/** Set the model directory and load the file.
	 * @param	directory	the model directory */
	void setModelDirectory(const std::string & directory);
	/** Set the model skin index to use.
	 * @param	index		the index to use */
	void setSkin(const unsigned int & index);
	/** Set the model animation index to use.
	 * @param	index		the index to use */
	void setAnimation(const unsigned int & index);
	/** Set the transformation for this model.
	 * @param	transform	the transform to use */
	void setTransform(const Transform & transform);
};

class Anim_Model_Creator : public ComponentCreator
{
public:
	Anim_Model_Creator() : ComponentCreator() {}
	virtual Component* create(Engine *engine) {
		return new Model_Animated_C(engine);
	}
};

#endif // ANIM_MODEL_COMPONENT_H