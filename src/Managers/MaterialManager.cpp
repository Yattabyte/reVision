#include "Managers\MaterialManager.h"


MaterialManager::~MaterialManager() 
{
	unique_lock<shared_mutex> writeGuard(m_DataMutex);
	if (m_Initialized) {
		m_FreeSpots.clear();
		m_Initialized = false;
	}
	/** @todo destructor */
}

MaterialManager::MaterialManager()
{
	m_Initialized = false;
	m_Count = 0;	
}

void MaterialManager::initialize()
{
	unique_lock<shared_mutex> writeGuard(m_DataMutex);
	if (!m_Initialized) {
		m_buffer = new DynamicBuffer();
		m_buffer->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 0);
		m_Initialized = true;
	}
}

void MaterialManager::bind()
{
	shared_lock<shared_mutex> readGuard(m_DataMutex);
	m_buffer->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 0);
}

GLuint MaterialManager::generateID()
{
	unique_lock<shared_mutex> writeGuard(m_DataMutex);
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

	unique_lock<shared_mutex> writeGuard(m_DataMutex);
	m_buffer->write(sizeof(GLuint64) * materialightingFBOID, sizeof(GLuint64), &handle);
	m_WorkOrders.push_back(handle);
}

void MaterialManager::parseWorkOrders()
{
	// Parse work orders
	unique_lock<shared_mutex> writeGuard(m_DataMutex);
	for each (const auto &handle in m_WorkOrders)
		glMakeTextureHandleResidentARB(handle);
	m_WorkOrders.clear();
}
