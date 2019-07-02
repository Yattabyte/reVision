#pragma once
#ifndef GEOMETRY_TECHNIQUE_H
#define GEOMETRY_TECHNIQUE_H

#include "Modules/Graphics/Common/Graphics_Technique.h"


class Geometry_Technique : public Graphics_Technique {
public:
	// Public (de)Constructors
	/***/
	inline ~Geometry_Technique() = default;
	/***/
	inline Geometry_Technique() : Graphics_Technique(GEOMETRY) {}


	// Public Interface Declaration
	inline virtual void renderTechnique(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::shared_ptr<CameraBuffer> & camera) override {
		renderGeometry(deltaTime, viewport, camera);
	}
	/***/
	inline virtual void renderGeometry(const float & deltaTime, const std::shared_ptr<Viewport> & viewport, const std::shared_ptr<CameraBuffer> & camera) {}
	/***/
	inline virtual void renderShadows(const float & deltaTime, const std::shared_ptr<CameraBuffer> & camera, const int & layer, const glm::vec3 & finalColor) {}
};

#endif // GEOMETRY_TECHNIQUE_H