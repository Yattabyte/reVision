#pragma once
#ifndef DIRECTDATA_H
#define DIRECTDATA_H

#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Common/Camera.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowData.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <memory>
#include <vector>




/** Structure to contain data that changes frame-to-frame, for direct light rendering. */
struct Direct_Light_Data {
	/** OpenGL buffer struct for directional lights. */
	constexpr const static int MAX_PERSPECTIVE_ARRAY = 6;
	struct Direct_Light_Buffer {
		glm::mat4 lightVP[MAX_PERSPECTIVE_ARRAY];
		glm::mat4 mMatrix = glm::mat4(1.0f);
		glm::vec3 LightColor = glm::vec3(1.0f); float padding1;
		glm::vec3 LightPosition = glm::vec3(0.0f); float padding2;
		glm::vec3 LightDirection = glm::vec3(0, -1, 0); float padding3;
		float CascadeEndClipSpace[MAX_PERSPECTIVE_ARRAY];
		float LightIntensity = 1.0f;
		float LightRadius = 1.0f;
		float LightCutoff = 1.0f;
		int Shadow_Spot = -1;
		int Light_Type = 0;  int padding4;
	};
	/** Struct collating per-perspective data. */
	struct Direct_Light_ViewInfo {
		std::vector<GLint> lightIndices;
		std::vector<Light_Component::Light_Type> lightTypes;
	};

	GL_ArrayBuffer<Direct_Light_Buffer> lightBuffer;
	std::vector<Direct_Light_ViewInfo> viewInfo;
	std::shared_ptr<Camera> clientCamera;
	std::shared_ptr<ShadowData> shadowData;
};

#endif DIRECTDATA_H