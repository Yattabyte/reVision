#pragma once
#ifndef SPOTDATA_H
#define SPOTDATA_H

#include "Modules/Graphics/Common/Camera.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Shadow/ShadowData.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <memory>
#include <vector>


/** Structure to contain data that changes frame-to-frame, for spot light rendering. */
struct SpotData {
	/** OpenGL buffer struct for spot lights. */
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
	/** Struct collating per-perspective data. */
	struct ViewInfo {
		std::vector<GLint> lightIndices;
	};

	GL_ArrayBuffer<Spot_Buffer> lightBuffer;
	std::vector<ViewInfo> viewInfo;
	std::shared_ptr<ShadowData> shadowData;
};

#endif SPOTDATA_H