#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include <algorithm>

/* -----ASSET TYPE----- */
#define ASSET_TYPE -1


Asset::~Asset()
{
}

Asset::Asset(const string & filename)
{	
	m_finalized = false;
	m_filename = filename;
}

int Asset::Get_Asset_Type() 
{
	return ASSET_TYPE;
}

string Asset::getFileName() const
{
	return m_filename;
}

void Asset::setFileName(const string & fn)
{
	m_filename = fn;
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
		Asset_Manager::Queue_Notification(m_observers); // Notify later, guaranteed to be done during rendering loop
	}
}

void Asset::addObserver(Asset_Observer * observer)
{
	unique_lock<shared_mutex> write_guard(m_mutex);
	m_observers.push_back(observer);
	write_guard.unlock();
	write_guard.release();
	shared_lock<shared_mutex> read_guard(m_mutex);
	if (m_finalized) // If we finalized already, new observer needs to know this is ready to go
		Asset_Manager::Queue_Notification(m_observers);
}

void Asset::removeObserver(Asset_Observer * observer)
{
	m_observers.erase(std::remove_if(begin(m_observers), end(m_observers), [observer](const auto *element) {
		return (element == observer);
	}), end(m_observers));
}