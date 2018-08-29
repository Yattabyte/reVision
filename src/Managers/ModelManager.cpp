#include "Managers\ModelManager.h"
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))


ModelManager::~ModelManager()
{
	glDeleteBuffers(NUM_VERTEX_ATTRIBUTES, m_vboIDS);
	glDeleteVertexArrays(1, &m_vaoID);
}

ModelManager::ModelManager()
{
	m_vaoID = 0;
	m_maxCapacity = 4000;
	m_currentSize = 0;
	m_fence = nullptr;
	m_outOfDate = false;
	ZERO_MEM(m_vboIDS);
}

void ModelManager::initialize()
{
	// Create VBO's
	constexpr GLbitfield flags = GL_DYNAMIC_STORAGE_BIT;// | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	glCreateBuffers(NUM_VERTEX_ATTRIBUTES, m_vboIDS);
	glNamedBufferStorage(m_vboIDS[0], m_maxCapacity * sizeof(glm::vec3), 0, flags);
	glNamedBufferStorage(m_vboIDS[1], m_maxCapacity * sizeof(glm::vec3), 0, flags);
	glNamedBufferStorage(m_vboIDS[2], m_maxCapacity * sizeof(glm::vec3), 0, flags);
	glNamedBufferStorage(m_vboIDS[3], m_maxCapacity * sizeof(glm::vec3), 0, flags);
	glNamedBufferStorage(m_vboIDS[4], m_maxCapacity * sizeof(glm::vec2), 0, flags);
	glNamedBufferStorage(m_vboIDS[5], m_maxCapacity * sizeof(VertexBoneData), 0, flags);

	// Create VAO
	glCreateVertexArrays(1, &m_vaoID);
	for (unsigned int x = 0; x < NUM_VERTEX_ATTRIBUTES; ++x)
		glEnableVertexArrayAttrib(m_vaoID, x);
	glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(m_vaoID, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(m_vaoID, 2, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(m_vaoID, 3, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(m_vaoID, 4, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribIFormat(m_vaoID, 5, 4, GL_INT, 0);
	glVertexArrayAttribFormat(m_vaoID, 6, 4, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(m_vaoID, 0, m_vboIDS[0], 0, 12);
	glVertexArrayVertexBuffer(m_vaoID, 1, m_vboIDS[1], 0, 12);
	glVertexArrayVertexBuffer(m_vaoID, 2, m_vboIDS[2], 0, 12);
	glVertexArrayVertexBuffer(m_vaoID, 3, m_vboIDS[3], 0, 12);
	glVertexArrayVertexBuffer(m_vaoID, 4, m_vboIDS[4], 0, 8);
	glVertexArrayVertexBuffer(m_vaoID, 5, m_vboIDS[5], 0, 32);
	glVertexArrayAttribBinding(m_vaoID, 0, 0);
	glVertexArrayAttribBinding(m_vaoID, 1, 1);
	glVertexArrayAttribBinding(m_vaoID, 2, 2);
	glVertexArrayAttribBinding(m_vaoID, 3, 3);
	glVertexArrayAttribBinding(m_vaoID, 4, 4);
	glVertexArrayAttribBinding(m_vaoID, 5, 5);
	glVertexArrayAttribBinding(m_vaoID, 6, 6);

	if (m_fence) {
		glDeleteSync(m_fence);
		m_fence = nullptr;
	}
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void ModelManager::registerGeometry(const GeometryInfo & data, size_t & offset, size_t & count)
{
	const size_t arraySize = data.vs.size();
	expandToFit(arraySize);

	{
		std::shared_lock<std::shared_mutex> read_guard(m_mutex);
		offset = m_currentSize;
		count = arraySize;
		// No need to check fence, since we are writing to a NEW range
		glNamedBufferSubData(m_vboIDS[0], m_currentSize * sizeof(glm::vec3), arraySize * sizeof(glm::vec3), &data.vs[0][0]);
		glNamedBufferSubData(m_vboIDS[1], m_currentSize * sizeof(glm::vec3), arraySize * sizeof(glm::vec3), &data.nm[0][0]);
		glNamedBufferSubData(m_vboIDS[2], m_currentSize * sizeof(glm::vec3), arraySize * sizeof(glm::vec3), &data.tg[0][0]);
		glNamedBufferSubData(m_vboIDS[3], m_currentSize * sizeof(glm::vec3), arraySize * sizeof(glm::vec3), &data.bt[0][0]);
		glNamedBufferSubData(m_vboIDS[4], m_currentSize * sizeof(glm::vec2), arraySize * sizeof(glm::vec2), &data.uv[0][0]);
		glNamedBufferSubData(m_vboIDS[5], m_currentSize * sizeof(VertexBoneData), arraySize * sizeof(VertexBoneData), &data.bones[0]);
	}
	
	std::unique_lock<std::shared_mutex> write_guard(m_mutex);
	m_currentSize += arraySize;
	if (m_fence) {
		glDeleteSync(m_fence);
		m_fence = nullptr;
	}
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
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
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboIDS[0], 0, 12);
		glVertexArrayVertexBuffer(m_vaoID, 1, m_vboIDS[1], 0, 12);
		glVertexArrayVertexBuffer(m_vaoID, 2, m_vboIDS[2], 0, 12);
		glVertexArrayVertexBuffer(m_vaoID, 3, m_vboIDS[3], 0, 12);
		glVertexArrayVertexBuffer(m_vaoID, 4, m_vboIDS[4], 0, 8);
		glVertexArrayVertexBuffer(m_vaoID, 5, m_vboIDS[5], 0, 32);

		read_guard.unlock();
		read_guard.release();
		std::unique_lock<std::shared_mutex> write_guard(m_mutex);
		m_outOfDate = false;
	}
}

void ModelManager::expandToFit(const size_t & arraySize)
{
	std::unique_lock<std::shared_mutex> write_guard(m_mutex);

	// Check if we can fit the desired data
	if (m_currentSize + arraySize > m_maxCapacity) {
		// Create new set of VBO's large enough to fit old data + desired data
		m_maxCapacity += arraySize * 2;

		// Create the new VBO's
		constexpr GLbitfield flags = GL_DYNAMIC_STORAGE_BIT;// | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		GLuint newVBOIDS[NUM_VERTEX_ATTRIBUTES];
		glCreateBuffers(NUM_VERTEX_ATTRIBUTES, newVBOIDS);
		glNamedBufferStorage(newVBOIDS[0], m_maxCapacity * sizeof(glm::vec3), 0, flags);
		glNamedBufferStorage(newVBOIDS[1], m_maxCapacity * sizeof(glm::vec3), 0, flags);
		glNamedBufferStorage(newVBOIDS[2], m_maxCapacity * sizeof(glm::vec3), 0, flags);
		glNamedBufferStorage(newVBOIDS[3], m_maxCapacity * sizeof(glm::vec3), 0, flags);
		glNamedBufferStorage(newVBOIDS[4], m_maxCapacity * sizeof(glm::vec2), 0, flags);
		glNamedBufferStorage(newVBOIDS[5], m_maxCapacity * sizeof(VertexBoneData), 0, flags);

		// Copy old VBO's
		glCopyNamedBufferSubData(m_vboIDS[0], newVBOIDS[0], 0, 0, m_currentSize * sizeof(glm::vec3));
		glCopyNamedBufferSubData(m_vboIDS[1], newVBOIDS[1], 0, 0, m_currentSize * sizeof(glm::vec3));
		glCopyNamedBufferSubData(m_vboIDS[2], newVBOIDS[2], 0, 0, m_currentSize * sizeof(glm::vec3));
		glCopyNamedBufferSubData(m_vboIDS[3], newVBOIDS[3], 0, 0, m_currentSize * sizeof(glm::vec3));
		glCopyNamedBufferSubData(m_vboIDS[4], newVBOIDS[4], 0, 0, m_currentSize * sizeof(glm::vec2));
		glCopyNamedBufferSubData(m_vboIDS[5], newVBOIDS[5], 0, 0, m_currentSize * sizeof(VertexBoneData));

		// Delete the old VBO's
		glDeleteBuffers(NUM_VERTEX_ATTRIBUTES, m_vboIDS);

		// Overwrite old VBO ID's
		for (int x = 0; x < NUM_VERTEX_ATTRIBUTES; ++x)
			m_vboIDS[x] = newVBOIDS[x];
		m_outOfDate = true;
		if (m_fence) {
			glDeleteSync(m_fence);
			m_fence = nullptr;
		}
		m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	}
}

const GLuint & ModelManager::getVAO() const
{
	return m_vaoID;
}

const bool ModelManager::finishedWork()
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

