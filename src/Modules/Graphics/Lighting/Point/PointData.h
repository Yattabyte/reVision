#pragma once
#ifndef POINTDATA_H
#define POINTDATA_H

#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowData.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <memory>
#include <vector>


/***/
struct PointData {
	/** OpenGL buffer for point lights. */
	struct Point_Buffer {
		glm::mat4 shadowVP[6];
		glm::mat4 mMatrix;
		glm::vec3 LightColor; float padding1;
		glm::vec3 LightPosition; float padding2;
		float LightIntensity;
		float LightRadius;
		int Shadow_Spot; float padding3;
	};
	/***/
	struct ViewInfo {
		size_t visLightCount = 0ull;
		DynamicBuffer visLights;
		StaticBuffer indirectShape = StaticBuffer(sizeof(GLuint) * 4);
	};

	GL_ArrayBuffer<Point_Buffer> lightBuffer;
	std::vector<ViewInfo> viewInfo;
	size_t shapeVertexCount = 0ull;
	std::shared_ptr<ShadowData> shadowData;
};

#endif POINTDATA_H