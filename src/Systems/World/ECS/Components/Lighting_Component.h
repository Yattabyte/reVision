#pragma once
#ifndef LIGHTING_COMPONENT
#define LIGHTING_COMPONENT
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECS\Components\Component.h"
#include "Systems\World\Visibility_Token.h"
#include "glm\glm.hpp"

using namespace glm;
class VB_Ptr;


/**
 * An interface for lighting related components.
 **/
class DT_ENGINE_API Lighting_Component : protected Component
{
public:
	// Public Methods
	/** Renders the light, performing shadow calculations only. */
	virtual void shadowPass() {};
	/** Tests if this object is within the viewing frustum of the camera.
	 * @brief				a test of general visibility (excluding obstruction of other objects). 
	 * @param	PMatrix		the projection matrix of the camera
	 * @param	VMatrix		the viewing matrix of the camera
	 * @return				true if this object is within the viewing frustum of the camera, false otherwise */
	virtual bool isVisible(const mat4 & PMatrix, const mat4 &VMatrix) = 0;
	/** Generates an 'importance' value for this light.
	 * @brief				importance is equal to (distance / size).
	 * @param	position	the position of the viewer (camera)
	 * @return				the importance value calculated. */
	virtual float getImportance(const vec3 &position) const = 0;
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
	/** Constructor. Takes in component ID and parent ID. */
	Lighting_Component(const ECShandle & id, const ECShandle & pid) : Component(id, pid) { m_shadowUpdateTime = 0; };


	// Protected Attributes
	double m_shadowUpdateTime;
	unsigned int m_uboIndex;
	VB_Ptr * m_uboBuffer;
};

#endif // LIGHTING_COMPONENT