#pragma once
#ifndef REFLECTORDATA_H
#define REFLECTORDATA_H

#include "Modules/Graphics/Common/Camera.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Modules/Graphics/Lighting/Reflector/FBO_Env_Reflector.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <memory>
#include <vector>


/** Structure to contain data that changes frame-to-frame, for reflector rendering. */
struct ReflectorData {
	/** OpenGL buffer struct for reflector lights. */
	struct Reflector_Buffer {
		glm::mat4 mMatrix;
		glm::mat4 rotMatrix;
		glm::vec3 BoxCamPos; float padding1;
		glm::vec3 BoxScale; float padding2;
		int CubeSpot; glm::vec3 padding3;
	};
	/** Struct collating per-perspective data. */
	struct ViewInfo {
		std::vector<GLint> lightIndices;
	};

	GL_ArrayBuffer<Reflector_Buffer> lightBuffer;
	std::vector<ViewInfo> viewInfo;
	FBO_Env_Reflector envmapFBO;
	std::vector<std::tuple<float, float*, int, Camera*>> reflectorsToUpdate;
	glm::ivec2 envmapSize = glm::ivec2(1);
	size_t reflectorLayers = 0ull;
};

#endif REFLECTORDATA_H