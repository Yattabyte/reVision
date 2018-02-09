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
		glGenBuffers(1, &m_BufferSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_BufferSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Material_Buffer), &m_MatBuffer, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_BufferSSBO);
		GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
		memcpy(p, &m_MatBuffer, sizeof(Material_Buffer));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		m_Initialized = true;
	}
}

void Material_Manager::_shutdown()
{
	unique_lock<shared_mutex> writeGuard(m_DataMutex);
	if (m_Initialized) {
		glDeleteBuffers(1, &m_BufferSSBO);
		m_FreeSpots.clear();
		m_MatBuffer = Material_Buffer();
		m_BufferSSBO = 0;
		m_Initialized = false;
	}
}

GLuint Material_Manager::GenerateMaterialBufferID()
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

void Material_Manager::GenerateHandle(const GLuint &materialBufferID, const GLuint &glTextureID)
{
	auto &manager = Get();
	unique_lock<shared_mutex> writeGuard(manager.m_DataMutex);
	GLuint64 handle = glGetTextureHandleARB(glTextureID);
	glMakeTextureHandleResidentARB(handle);
	glBindBuffer(GL_UNIFORM_BUFFER, manager.m_BufferSSBO);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(GLuint64) * materialBufferID, sizeof(GLuint64), &handle);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	manager.m_MatBuffer.MaterialMaps[materialBufferID] = handle;	
	manager.m_WorkOrders.push_back(handle);
}

void Material_Manager::ParseWorkOrders()
{
	auto &manager = Get();

	unique_lock<shared_mutex> writeGuard(manager.m_DataMutex);
	for each (const auto &handle in manager.m_WorkOrders) 
		glMakeTextureHandleResidentARB(handle);	
	manager.m_WorkOrders.clear();
}