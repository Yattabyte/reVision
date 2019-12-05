#pragma once
#ifndef GEOMETRY_TECHNIQUE_H
#define GEOMETRY_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"


/** A rendering technique category responsible for displaying and shadowing geometry. */
class Geometry_Technique : public Graphics_Technique {
public:
	// Public (De)Constructors
	/** Destroy this technique. */
	inline ~Geometry_Technique() = default;
	/** Construct this technique. */
	Geometry_Technique() noexcept;


	// Public Interface Implementation
	virtual void renderTechnique(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept override;


	// Public Interface Declarations
	/** Apply this geometry technique.
	@param	deltaTime		the amount of time passed since last frame.
	@param	viewport		the viewport to render from. */
	virtual void renderGeometry(const float& deltaTime, const std::shared_ptr<Viewport>& viewport, const std::vector<std::pair<int, int>>& perspectives) noexcept;
	/** Use geometry techniques to cull shadows.
	@param	deltaTime		the amount of time passed since last frame.
	@param	perspectives	the camera and layer indices to render. */
	virtual void cullShadows(const float& deltaTime, const std::vector<std::pair<int, int>>& perspectives) noexcept;
	/** Use geometry techniques to render shadows.
	@param	deltaTime		the amount of time passed since last frame. */
	virtual void renderShadows(const float& deltaTime) noexcept;
};

#endif // GEOMETRY_TECHNIQUE_H