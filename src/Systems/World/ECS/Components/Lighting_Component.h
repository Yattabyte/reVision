#pragma once
#ifndef LIGHTING_COMPONENT_H
#define LIGHTING_COMPONENT_H

#include "Systems\World\ECS\Components\Component.h"
#include "Systems\World\Visibility_Token.h"
#include "glm\glm.hpp"

using namespace glm;
class VB_Ptr;


/**
 * An interface for lighting related components.
 **/
class Lighting_Component : protected Component
{
public:
	// Public Methods
	/** Generates an 'importance' value for this light.
	 * @brief				importance is equal to (distance / size).
	 * @param	position	the position of the viewer (camera)
	 * @return				the importance value calculated. */
	virtual float getImportance(const vec3 & position) const = 0;
	/** Renders the light, performing occlusion calculations only. */
	virtual void occlusionPass(const unsigned int & type) {};
	/** Renders the light, performing shadow calculations only. */
	virtual void shadowPass(const unsigned int & type) {};
	/** Update this component, and send updated data to the GPU. */
	virtual void update(const unsigned int & type) = 0;
	/** Retrieves the timestamp of the last shadowmap update.
	 * @return				the shadow update time */
	const double getShadowUpdateTime() const { return m_shadowUpdateTime; }
	/** Retrieve the buffer index for this light.
	 * @return	the buffer index */
	const unsigned int getBufferIndex() const {	return m_uboIndex; }


protected:
	// (de)Constructors
	/** Virtual Destructor. */
	~Lighting_Component() {};
	/** Constructor. */
	Lighting_Component() { m_shadowUpdateTime = 0; };


	// Protected Attributes
	double m_shadowUpdateTime;
	unsigned int m_uboIndex;
	VB_Ptr * m_uboBuffer;
};

#endif // LIGHTING_COMPONENT_H