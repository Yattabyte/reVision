#pragma once
#ifndef GEOMETRY_TECHNIQUE_H
#define GEOMETRY_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"


/** A rendering technique category responsible for displaying and shadowing geometry. */
class Geometry_Technique : public Graphics_Technique {
public:
	// Public (de)Constructors
	/** Destroy this technique. */
	inline ~Geometry_Technique() = default;
	/** Construct this technique. */
	inline Geometry_Technique() : Graphics_Technique(GEOMETRY) {}


	// Public Interface Implementation
	inline virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::shared_ptr<RH_Volume>& rhVolume, const std::vector<std::pair<int, int>>& perspectives) override {
		// Forward to geometry rendering
		renderGeometry(deltaTime, viewport, perspectives);
	}


	// Public Interface Declarations
	/** Apply this geometry technique.
	@param	deltaTime		the amount of time passed since last frame.
	@param	viewport		the viewport to render from. */
	inline virtual void renderGeometry(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) {}
	/** Use geometry techniques to cull shadows.
	@param	deltaTime		the amount of time passed since last frame.
	@param	perspectives	the camera and layer indicies to render. */
	inline virtual void cullShadows(const float& deltaTime, const std::vector<std::pair<int, int>>& perspectives) {}
	/** Use geometry techniques to render shadows.
	@param	deltaTime		the amount of time passed since last frame. */
	inline virtual void renderShadows(const float& deltaTime) {}
};

#endif // GEOMETRY_TECHNIQUE_H