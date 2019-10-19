#pragma once
#ifndef POINTDATA_H
#define POINTDATA_H

#include "Modules/Graphics/Common/Camera.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowData.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/GL_Vector.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <memory>
#include <vector>


/** Structure to contain data that changes frame-to-frame, for point light rendering. */
struct PointData {
	/** OpenGL buffer struct for point lights. */
	struct Point_Buffer {
		glm::mat4 shadowVP[6];
		glm::mat4 mMatrix;
		glm::vec3 LightColor; float padding1;
		glm::vec3 LightPosition; float padding2;
		float LightIntensity;
		float LightRadius;
		int Shadow_Spot; float padding3;
	};
	/** Struct collating per-perspective data. */
	struct ViewInfo {
		std::vector<GLint> lightIndices;
	};

	GL_Vector<Point_Buffer> lightBuffer;
	std::vector<ViewInfo> viewInfo;
	std::shared_ptr<ShadowData> shadowData;
};

#endif POINTDATA_H