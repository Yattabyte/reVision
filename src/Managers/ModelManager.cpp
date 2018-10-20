#include "Managers\ModelManager.h"


ModelManager::~ModelManager()
{
	glDeleteBuffers(1, &m_vboID);
	glDeleteVertexArrays(1, &m_vaoID);
}

void ModelManager::initialize()
{
	// Create VBO's
	glCreateBuffers(1, &m_vboID);
	glNamedBufferStorage(m_vboID, m_maxCapacity * sizeof(SingleVertex), 0, GL_DYNAMIC_STORAGE_BIT);
	// Create VAO
	glCreateVertexArrays(1, &m_vaoID);
	// Enable 7 attribute locations which all source data from binding point 0
	for (unsigned int x = 0; x < NUM_VERTEX_ATTRIBUTES; ++x) {
		glEnableVertexArrayAttrib(m_vaoID, x);
		glVertexArrayAttribBinding(m_vaoID, x, 0);
	}
	// Specify how the data should be broken up, and into what layout locations in the shaders (7 attribute locations)
	glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, vertex));
	glVertexArrayAttribFormat(m_vaoID, 1, 3, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, normal));
	glVertexArrayAttribFormat(m_vaoID, 2, 3, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, tangent));
	glVertexArrayAttribFormat(m_vaoID, 3, 3, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, bitangent));
	glVertexArrayAttribFormat(m_vaoID, 4, 2, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, uv));
	glVertexArrayAttribIFormat(m_vaoID, 5, 1, GL_INT, offsetof(SingleVertex, matID));
	glVertexArrayAttribIFormat(m_vaoID, 6, 4, GL_INT, offsetof(SingleVertex, boneIDs));
	glVertexArrayAttribFormat(m_vaoID, 7, 4, GL_FLOAT, GL_FALSE, offsetof(SingleVertex, weights));
	// Specify data from the one vertex buffer to binding point 0
	glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(SingleVertex));	
}

void ModelManager::registerGeometry(const GeometryInfo & data, size_t & offset, size_t & count)
{
	std::unique_lock<std::shared_mutex> write_guard(m_mutex);
	const size_t arraySize = data.m_vertices.size();
	expandToFit(arraySize);
	
	offset = m_currentSize;
	count = arraySize;
	// No need to check fence, since we are writing to a NEW range
	glNamedBufferSubData(m_vboID, m_currentSize * sizeof(SingleVertex), arraySize * sizeof(SingleVertex), &data.m_vertices[0]);	
	
	m_currentSize += arraySize;
	if (m_fence) 
		glDeleteSync(m_fence);	
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	m_changed = true;
}

void ModelManager::unregisterGeometry(const GeometryInfo & data, const size_t & offset, const size_t & count)
{
	/**@todo remove geometry*/
}

void ModelManager::update()
{
	std::shared_lock<std::shared_mutex> read_guard(m_mutex);
	if (m_outOfDate) {
		// Assign VAO to new VBO's
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(SingleVertex));

		read_guard.unlock();
		read_guard.release();
		std::unique_lock<std::shared_mutex> write_guard(m_mutex);
		m_outOfDate = false;
	}
}

void ModelManager::expandToFit(const size_t & arraySize)
{
	// Check if we can fit the desired data
	if (m_currentSize + arraySize > m_maxCapacity) {
		if (m_fence) {
			// Wait for data fence to be passed
			GLenum state = GL_UNSIGNALED;
			while (state != GL_SIGNALED && state != GL_ALREADY_SIGNALED && state != GL_CONDITION_SATISFIED)
				state = glClientWaitSync(m_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
			glDeleteSync(m_fence);
			m_fence = nullptr;
		}

		// Create new set of VBO's large enough to fit old data + desired data
		m_maxCapacity += arraySize * 2;

		// Create the new VBO's
		GLuint newVBOID = 0;
		glCreateBuffers(1, &newVBOID);
		glNamedBufferStorage(newVBOID, m_maxCapacity * sizeof(SingleVertex), 0, GL_DYNAMIC_STORAGE_BIT);

		// Copy old VBO's
		glCopyNamedBufferSubData(m_vboID, newVBOID, 0, 0, m_currentSize * sizeof(SingleVertex));

		// Delete the old VBO's
		glDeleteBuffers(1, &m_vboID);

		// Overwrite old VBO ID's
		m_vboID = newVBOID;
		m_outOfDate = true;
	}
}

const GLuint & ModelManager::getVAO() const
{
	return m_vaoID;
}

const bool ModelManager::readyToUse()
{
	std::shared_lock<std::shared_mutex> readGuard(m_mutex);	
	if (!m_fence)
		return true;
	const GLenum & state = glClientWaitSync(m_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
	if (state == GL_SIGNALED || state == GL_ALREADY_SIGNALED || state == GL_CONDITION_SATISFIED) {
		glDeleteSync(m_fence);
		m_fence = nullptr;
		return true;
	}
	return false;
}

const bool ModelManager::hasChanged()
{
	// Changes every time another piece of geometry is added
	std::shared_lock<std::shared_mutex> readGuard(m_mutex);
	bool state = m_changed;
	m_changed = false;
	return state;
}


