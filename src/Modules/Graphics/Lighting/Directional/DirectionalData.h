#pragma once
#ifndef DIRECTIONALDATA_H
#define DIRECTIONALDATA_H

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

#define NUM_CASCADES 4


/***/
struct DirectionalData {
	/** OpenGL buffer for directional lights. */
	struct Directional_Buffer {
		glm::mat4 lightVP[NUM_CASCADES];
		float CascadeEndClipSpace[NUM_CASCADES];
		glm::vec3 LightColor = glm::vec3(1.0f); float padding1;
		glm::vec3 LightDirection = glm::vec3(0, -1, 0); float padding2;
		float LightIntensity = 1.0f;
		int Shadow_Spot = -1; glm::vec2 padding3;
	};
	/***/
	struct ViewInfo {
		size_t visShadowCount = 0ull;
		std::vector<GLint> lightIndices;
	};

	GL_ArrayBuffer<Directional_Buffer> lightBuffer;
	std::vector<ViewInfo> viewInfo;
	std::shared_ptr<CameraBuffer> clientCamera;
	std::shared_ptr<ShadowData> shadowData;
};

#endif DIRECTIONALDATA_H