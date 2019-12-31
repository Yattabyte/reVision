#pragma once
#ifndef PROPDATA_H
#define PROPDATA_H

#include "Utilities/GL/GL_Vector.h"
#include "glm/glm.hpp"
#include <vector>

#define NUM_MAX_BONES 100


/** Structure to contain data that changes frame-to-frame, for prop rendering. */
struct PropData {
	/** OpenGL buffer struct for models. */
	struct Model_Buffer {
		GLuint materialID;
		GLuint skinID; glm::vec2 padding1;
		glm::mat4 mMatrix;
		glm::mat4 bBoxMatrix;
	};
	/** OpenGL buffer struct for skeletons. */
	struct Skeleton_Buffer {
		glm::mat4 bones[NUM_MAX_BONES];
	};
	/** Struct collating per-perspective data. */
	struct ViewInfo {
		std::vector<glm::ivec4> cullingDrawData, renderingDrawData;
		std::vector<GLuint> visibleIndices;
		std::vector<int> skeletonData;
	};

	GL_Vector<Model_Buffer> modelBuffer;
	GL_Vector<Skeleton_Buffer> skeletonBuffer;
	std::vector<ViewInfo> viewInfo;
	GLuint m_geometryVAOID = 0u, m_materialArrayID = 0u;
};

#endif // PROPDATA_H