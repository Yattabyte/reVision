#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include <algorithm>


Asset::~Asset()
{
}

Asset::Asset(const string & filename)
{	
	m_finalized = false;
	m_filename = filename;
}

string Asset::getFileName() const
{
	return m_filename;
}

void Asset::setFileName(const string & fn)
{
	m_filename = fn;
}

void Asset::removeCallback(void * pointerID) 
{
	if (m_callbacks.find(pointerID) != m_callbacks.end())
		m_callbacks.erase(m_callbacks.find(pointerID));	
}

bool Asset::existsYet()
{ 
	shared_lock<shared_mutex> read_guard(m_mutex);
	return m_finalized;
}

void Asset::finalize()
{
	if (!m_finalized) {
		unique_lock<shared_mutex> write_guard(m_mutex);
		m_finalized = true;
		write_guard.unlock();
		write_guard.release();

		shared_lock<shared_mutex> read_guard(m_mutex);
		vector<function<void()>> funcs;
		funcs.reserve(m_callbacks.size());
		for each (const auto & func in m_callbacks)
			funcs.push_back(func.second);
		read_guard.unlock();
		read_guard.release();
		Asset_Manager::Queue_Notification(funcs);
	}
}