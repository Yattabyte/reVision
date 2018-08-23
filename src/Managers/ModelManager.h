#pragma once
#ifndef MODELMANAGER_H
#define MODELMANAGER_H
#define GLEW_STATIC
#define NUM_VERTEX_ATTRIBUTES 6

#include "Utilities\IO\Model_IO.h"
#include "GL\glew.h"
#include "glm\glm.hpp"
#include <shared_mutex>
#include <vector>


struct GeometryInfo;

/** A single storage point for all model geometry. Maintains a single vertex array object. */
class ModelManager {
public:
	// (de)Constructors
	/** Destroy the model manager. */
	~ModelManager();
	/** Construct the model manager. */
	ModelManager();

	
	// Public Functions
	/** Initialzie the model manager. */
	void initialize();
	/** Submit some continuous geometric data into this buffer. 
	@param	data	the data to submit
	@param	offset	the offset of the data (gets updated) 
	@param	count	the count of the data (gets updated) */
	void registerGeometry(const GeometryInfo & data, GLint &offset, GLint &count);
	/** Remove some continuous geometric data from this buffer. 
	@param	data	the data to remove
	@param	offset	the offset of the data 
	@param	count	the count of the data */
	void unregisterGeometry(const GeometryInfo & data, const GLint &offset, const GLint &count);
	/** Update the buffer's VAO from the main thread. */
	void update();
	/** Retreive this buffer's VAO ID */
	const GLuint & getVAO() const;
	/** Returns whether or not this manager has work left.
	@return	true if all work is finished, false otherwise. */
	const bool finishedWork();


private:
	// Private Functions
	/** Expand this buffer's underlying container (performs a copy operation and generates a new buffer). */
	void expandToFit(const size_t &arraySize);


	// Private Attributes
	GLuint m_vaoID;
	GLuint m_vboIDS[NUM_VERTEX_ATTRIBUTES];
	GLuint m_maxCapacity;
	GLuint m_currentSize;
	GLsync m_fence;
	bool m_outOfDate;	
	std::shared_mutex m_mutex;
};

struct GeometryInfo {
	std::vector<glm::vec3> vs, nm, tg, bt;
	std::vector<glm::vec2> uv;
	std::vector<VertexBoneData> bones;
};

#endif // MODELMANAGER_H