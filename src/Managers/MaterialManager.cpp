#include "Managers\MaterialManager.h"


MaterialManager::~MaterialManager() 
{
	std::unique_lock<std::shared_mutex> writeGuard(m_DataMutex);
	m_FreeSpots.clear();
}

void MaterialManager::bind()
{
	std::shared_lock<std::shared_mutex> readGuard(m_DataMutex);
	m_buffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 0);
}

GLuint MaterialManager::generateID()
{
	std::unique_lock<std::shared_mutex> writeGuard(m_DataMutex);
	GLuint arraySpot = m_Count;
	if (m_FreeSpots.size()) {
		arraySpot = m_FreeSpots.front();
		m_FreeSpots.pop_front();
	}
	else
		m_Count++;
	return arraySpot;
}

void MaterialManager::generateHandle(const GLuint & materialightingFBOID, const GLuint & glTextureID)
{
	const GLuint64 handle = glGetTextureHandleARB(glTextureID);

	std::unique_lock<std::shared_mutex> writeGuard(m_DataMutex);
	m_buffer.write(sizeof(GLuint64) * materialightingFBOID, sizeof(GLuint64), &handle);
	m_WorkOrders.push_back(handle);
}

void MaterialManager::parseWorkOrders()
{
	// Parse work orders
	std::unique_lock<std::shared_mutex> writeGuard(m_DataMutex);
	for each (const auto &handle in m_WorkOrders)
		glMakeTextureHandleResidentARB(handle);
	m_WorkOrders.clear();
}

const bool MaterialManager::finishedWork()
{
	std::shared_lock<std::shared_mutex> readGuard(m_DataMutex);
	return !bool(m_WorkOrders.size());
}
