#pragma once
#ifndef GEOMETRY_TECHNIQUE_H
#define GEOMETRY_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"


/***/
class Geometry_Technique : public Graphics_Technique {
public:
	// Public (de)Constructors
	/***/
	inline ~Geometry_Technique() = default;
	/***/
	inline Geometry_Technique() : Graphics_Technique(GEOMETRY) {}


	// Public Interface Implementation
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const CameraBuffer::CamStruct * camera) override {
		// Forward to geometry rendering
		renderGeometry(deltaTime, viewport, camera);
	}


	// Public Interface Declarations
	/***/
	inline virtual void renderGeometry(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const CameraBuffer::CamStruct * camera) {}
	/***/
	inline virtual void cullShadows(const float & deltaTime, const std::vector<std::pair<CameraBuffer::CamStruct*, int>> & perspectives) {}
	/***/
	inline virtual void renderShadows(const float & deltaTime) {}	
};

#endif // GEOMETRY_TECHNIQUE_H