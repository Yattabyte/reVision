#pragma once
#ifndef POINTDATA_H
#define POINTDATA_H

#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Point/FBO_Shadow_Point.h"
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
	FBO_Shadow_Point shadowFBO;
	std::vector<std::tuple<float*, int, std::vector<std::shared_ptr<CameraBuffer>>>> shadowsToUpdate;
	glm::ivec2 shadowSize = glm::ivec2(1);
	size_t shapeVertexCount = 0ull;
};

#endif POINTDATA_H