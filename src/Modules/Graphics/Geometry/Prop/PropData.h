#pragma once
#ifndef PROPDATA_H
#define PROPDATA_H

#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/component_wise.hpp"
#include <memory>
#include <vector>

#define NUM_MAX_BONES 100


/***/
struct PropData {
	/** OpenGL buffer for models. */
	struct Model_Buffer {
		GLuint materialID;
		GLuint isStatic; glm::vec2 padding1;
		glm::mat4 mMatrix;
		glm::mat4 bBoxMatrix;
	};
	/** OpenGL buffer for skeletons. */
	struct Skeleton_Buffer {
		glm::mat4 bones[NUM_MAX_BONES];
	};
	/***/
	struct ViewInfo {
		GLsizei visProps = 0;
		DynamicBuffer bufferPropIndex, bufferCulling, bufferRender, bufferSkeletonIndex;
	};

	GL_ArrayBuffer<Model_Buffer> modelBuffer;
	GL_ArrayBuffer<Skeleton_Buffer> skeletonBuffer;
	std::vector<ViewInfo> viewInfo;
};

#endif PROPDATA_H