#include "Managers\Material_Manager.h"


Material_Manager::Material_Manager()
{
	m_Initialized = false;
	m_Count = 0;
}

void Material_Manager::_startup()
{
	unique_lock<shared_mutex> writeGuard(m_DataMutex);
	if (!m_Initialized) {
		m_buffer = new DynamicBuffer();
		m_buffer->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 0);
		m_Initialized = true;
	}
}

void Material_Manager::_shutdown()
{
	unique_lock<shared_mutex> writeGuard(m_DataMutex);
	if (m_Initialized) {
		m_FreeSpots.clear();
		m_Initialized = false;
	}
}

void Material_Manager::Bind()
{
	auto &manager = Get();
	shared_lock<shared_mutex> readGuard(manager.m_DataMutex);
	manager.m_buffer->bindBufferBase(GL_SHADER_STORAGE_BUFFER, 0);
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

void Material_Manager::Generate_Handle(const GLuint & materialightingFBOID, const GLuint & glTextureID)
{
	auto &manager = Get();
	const GLuint64 handle = glGetTextureHandleARB(glTextureID);

	unique_lock<shared_mutex> writeGuard(manager.m_DataMutex);
	manager.m_buffer->write(sizeof(GLuint64) * materialightingFBOID, sizeof(GLuint64), &handle);
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