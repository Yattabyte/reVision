#pragma once
#ifndef MODELMANAGER_H
#define MODELMANAGER_H
#define NUM_VERTEX_ATTRIBUTES 8

#include "Utilities\IO\Mesh_IO.h"
#include "GL\glad\glad.h"
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
	ModelManager() = default;

	
	// Public Functions
	/** Initialzie the model manager. */
	void initialize();
	/** Submit some continuous geometric data into this buffer. 
	@param	data	the data to submit
	@param	offset	the offset of the data (gets updated) 
	@param	count	the count of the data (gets updated) */
	void registerGeometry(const GeometryInfo & data, size_t & offset, size_t & count);
	/** Remove some continuous geometric data from this buffer. 
	@param	data	the data to remove
	@param	offset	the offset of the data 
	@param	count	the count of the data */
	void unregisterGeometry(const GeometryInfo & data, const size_t & offset, const size_t & count);
	/** Update the buffer's VAO from the main thread. */
	void update();
	/** Retreive this buffer's VAO ID */
	const GLuint & getVAO() const;
	/** Returns whether or not this manager is ready to use.
	@return			true if all work is finished, false otherwise. */
	const bool readyToUse();
	/** Returns whether or not any changes have occured to this manager since the last check
	@return			true if any changes occured, false otherwise */
	const bool hasChanged();


private:
	// Private Functions
	/** Expand this buffer's underlying container (performs a copy operation and generates a new buffer). */
	void expandToFit(const size_t &arraySize);


	// Private Attributes
	GLuint m_vaoID = 0;
	GLuint m_vboID = 0;
	size_t m_maxCapacity = 4000;
	size_t m_currentSize = 0;
	GLsync m_fence = nullptr;
	bool m_outOfDate = false;
	bool m_changed = true;
	std::shared_mutex m_mutex;
};

#endif // MODELMANAGER_H