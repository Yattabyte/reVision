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

void Asset::removeCallback(const Asset_State & state, void * pointerID) {
	if (m_callbacks.find(state) != m_callbacks.end()) {
		auto &specific_map = m_callbacks[state];
		if (specific_map.find(pointerID) != specific_map.end())
			specific_map.erase(specific_map.find(pointerID));
	}
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
		Queue_Notification(Asset_State::FINALIZED);
	}
}

void Asset::Queue_Notification(const Asset_State & state)
{
	vector<function<void()>> funcs;
	funcs.reserve(m_callbacks[state].size());
	for each (const auto & func in m_callbacks[state]) 
		funcs.push_back(func.second);	

	Asset_Manager::Queue_Notification(funcs);
}