#pragma once
#ifndef SPOTDATA_H
#define SPOTDATA_H

#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Spot/FBO_Shadow_Spot.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <memory>
#include <vector>


/***/
struct SpotData {
	/** OpenGL buffer for spot lights. */
	struct Spot_Buffer {
		glm::mat4 lightPV;
		glm::mat4 mMatrix;
		glm::vec3 LightColor; float padding1;
		glm::vec3 LightPosition; float padding2;
		glm::vec3 LightDirection; float padding3;
		float LightIntensity;
		float LightRadius;
		float LightCutoff;
		int Shadow_Spot;
	};
	/***/
	struct ViewInfo {
		size_t visLightCount = 0ull;
		DynamicBuffer visLights;
		StaticBuffer indirectShape = StaticBuffer(sizeof(GLuint) * 4);
	};

	GL_ArrayBuffer<Spot_Buffer> lightBuffer;
	std::vector<ViewInfo> viewInfo;
	FBO_Shadow_Spot shadowFBO;
	std::vector<std::tuple<float, LightSpot_Component*, std::shared_ptr<CameraBuffer>>> shadowsToUpdate;
	glm::ivec2 shadowSize = glm::ivec2(1);
	size_t shapeVertexCount = 0ull;
};

#endif SPOTDATA_H