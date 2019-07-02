#pragma once
#ifndef DIRECTIONALDATA_H
#define DIRECTIONALDATA_H

#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Directional/FBO_Shadow_Directional.h"
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
		size_t visLightCount = 0ull, visShadowCount = 0ull;
		DynamicBuffer visLights;
		StaticBuffer indirectShape = StaticBuffer(sizeof(GLuint) * 4), indirectBounce = StaticBuffer(sizeof(GLuint) * 4);
	};

	GL_ArrayBuffer<Directional_Buffer> lightBuffer;
	std::vector<ViewInfo> viewInfo;
	std::shared_ptr<CameraBuffer> clientCamera;
	FBO_Shadow_Directional shadowFBO;
	std::vector<std::tuple<float*, int, std::vector<std::shared_ptr<CameraBuffer>>>> shadowsToUpdate;
	glm::ivec2 shadowSize = glm::ivec2(1);
	size_t shapeVertexCount = 0ull;
};

#endif DIRECTIONALDATA_H