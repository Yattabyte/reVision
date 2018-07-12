#pragma once
#ifndef GEOMETRY_TECHNIQUE_H
#define GEOMETRY_TECHNIQUE_H

#include "Systems\World\Visibility_Token.h"


class Camera;

/**
 * A base class for geometry rendering techniques
 **/
class Geometry_Technique {
public:
	// (de)Constructors
	/** Virtual Destructor. */
	~Geometry_Technique() {};
	/** Constructor. */
	Geometry_Technique() {};


	// Public Interface Methods
	/** Pepare this technique ahead of time. */
	virtual void updateData(const std::vector<Camera*> & viewers) = 0;
	/** Apply occlusion-culling rendering passes. */
	virtual void occlusionCullBuffers(Camera & camera) = 0;
	/** Render geometry to the framebuffer using this technique. */
	virtual void renderGeometry(Camera & camera) = 0;
};

#endif // GEOMETRY_TECHNIQUE_H