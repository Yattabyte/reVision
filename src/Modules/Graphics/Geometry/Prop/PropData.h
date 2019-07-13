#pragma once
#ifndef PROPDATA_H
#define PROPDATA_H

#include "Modules/Graphics/Common/Camera.h"
#include "Modules/Graphics/Geometry/components.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "Utilities/IO/Mesh_IO.h"
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
		GLuint materialID; glm::vec3 padding1;
		glm::mat4 mMatrix;
		glm::mat4 bBoxMatrix;
	};
	/** OpenGL buffer for skeletons. */
	struct Skeleton_Buffer {
		glm::mat4 bones[NUM_MAX_BONES];
	};
	/***/
	struct ViewInfo {
		std::vector<glm::ivec4> cullingDrawData, renderingDrawData;
		std::vector<GLuint> visibleIndices;
		std::vector<int> skeletonData;
	};

	GL_ArrayBuffer<Model_Buffer> modelBuffer;
	GL_ArrayBuffer<Skeleton_Buffer> skeletonBuffer;
	std::vector<ViewInfo> viewInfo;
	GLuint m_geometryVAOID = 0;
};

#endif PROPDATA_H