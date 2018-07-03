#include "Assets\Asset.h"
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
	unique_lock<shared_mutex> write_guard(m_mutex);
	if (m_callbacks.find(pointerID) != m_callbacks.end())
		m_callbacks.erase(m_callbacks.find(pointerID));	
}

bool Asset::existsYet() const
{ 
	shared_lock<shared_mutex> read_guard(m_mutex);
	return m_finalized;
}

void Asset::finalize()
{
	unique_lock<shared_mutex> write_guard(m_mutex);
	m_finalized = true;	
}
