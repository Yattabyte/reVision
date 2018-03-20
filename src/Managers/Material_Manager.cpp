#include "Managers\Material_Manager.h"


Material_Manager::Material_Manager()
{
	m_Initialized = false;
	m_BufferSSBO = 0;
	m_Count = 0;
}

void Material_Manager::_startup()
{
	unique_lock<shared_mutex> writeGuard(m_DataMutex);
	if (!m_Initialized) {
		m_BufferSSBO = 0;
		glCreateBuffers(1, &m_BufferSSBO);
		glNamedBufferStorage(m_BufferSSBO, sizeof(Material_Buffer), &m_MatBuffer, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		m_bufferPtr = glMapNamedBufferRange(m_BufferSSBO, 0, sizeof(Material_Buffer), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_BufferSSBO);	
		m_Initialized = true;
	}
}

void Material_Manager::_shutdown()
{
	unique_lock<shared_mutex> writeGuard(m_DataMutex);
	if (m_Initialized) {
		glUnmapNamedBuffer(m_BufferSSBO);
		glDeleteBuffers(1, &m_BufferSSBO);
		m_FreeSpots.clear();
		m_MatBuffer = Material_Buffer();
		m_BufferSSBO = 0;
		m_Initialized = false;
	}
}

GLuint Material_Manager::Generate_ID()
{
	auto &manager = Get();
	unique_lock<shared_mutex> writeGuard(manager.m_DataMutex);
	GLuint arraySpot = manager.m_Count;
	deque<unsigned int> &m_FreeSpots = manager.m_FreeSpots;
	if (m_FreeSpots.size()) {
		arraySpot = m_FreeSpots.front();
		m_FreeSpots.pop_front();
	}
	else
		manager.m_Count++;
	return arraySpot;
}

void Material_Manager::Generate_Handle(const GLuint & materialBufferID, const GLuint & glTextureID)
{
	auto &manager = Get();
	unique_lock<shared_mutex> writeGuard(manager.m_DataMutex);
	GLuint64 handle = glGetTextureHandleARB(glTextureID);
	glMakeTextureHandleResidentARB(handle);
	
	GLuint64 * mappedBuffer = reinterpret_cast<GLuint64*>(manager.m_bufferPtr);
	mappedBuffer[materialBufferID] = handle;
	manager.m_MatBuffer.MaterialMaps[materialBufferID] = handle;	
	manager.m_WorkOrders.push_back(handle);
}

void Material_Manager::Parse_Work_Orders()
{
	auto &manager = Get();

	unique_lock<shared_mutex> writeGuard(manager.m_DataMutex);
	for each (const auto &handle in manager.m_WorkOrders) 
		glMakeTextureHandleResidentARB(handle);	
	manager.m_WorkOrders.clear();
}