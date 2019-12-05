#pragma once
#ifndef INDIRECTDATA_H
#define INDIRECTDATA_H

#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Common/Camera.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowData.h"
#include "Utilities/GL/GL_Vector.h"
#include "glm/glm.hpp"
#include <vector>


/** Structure to contain data that changes frame-to-frame, for indirect light rendering. */
struct Indirect_Light_Data {
	/** OpenGL buffer struct for direct lights. */
	constexpr const static int MAX_PERSPECTIVE_ARRAY = 6;
	struct Indirect_Light_Buffer {
		glm::mat4 lightVP[MAX_PERSPECTIVE_ARRAY];
		glm::vec3 LightColor = glm::vec3(1.0f); float padding1;
		glm::vec3 LightPosition = glm::vec3(0.0f); float padding2;
		float CascadeEndClipSpace[MAX_PERSPECTIVE_ARRAY];
		float LightIntensity = 1.0f;
		int Shadow_Spot = -1;
		int Light_Type = 0; glm::vec3 padding3;
	};
	/** Struct collating per-perspective data. */
	struct Indirect_Light_ViewInfo {
		std::vector<GLint> lightIndices;
	};
	/** Construct this structure. */
	Indirect_Light_Data(ShadowData& sd, Camera& c) : shadowData(sd), clientCamera(c) {}

	ShadowData& shadowData;
	Camera& clientCamera;
	GL_Vector<Indirect_Light_Buffer> lightBuffer;
	std::vector<Indirect_Light_ViewInfo> viewInfo;
};

#endif INDIRECTDATA_H