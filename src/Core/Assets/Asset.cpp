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

int Asset::GetAssetType() 
{
	return ASSET_TYPE;
}

string Asset::GetFileName() const
{
	return m_filename;
}

void Asset::SetFileName(const string & fn)
{
	m_filename = fn;
}

bool Asset::ExistsYet()
{ 
	shared_lock<shared_mutex> read_guard(m_mutex);
	return m_finalized;
}

void Asset::Finalize()
{
	if (!m_finalized) {
		m_finalized = true;
		Asset_Manager::Queue_Notification(m_observers); // Notify later, guaranteed to be done during rendering loop
	}
}

void Asset::AddObserver(Asset_Observer * observer)
{
	unique_lock<shared_mutex> write_guard(m_mutex);
	m_observers.push_back(observer);
	if (m_finalized) // If we finalized already, new observer needs to know this is ready to go
		Asset_Manager::Queue_Notification(m_observers);
}

void Asset::RemoveObserver(Asset_Observer * observer)
{
	m_observers.erase(std::remove_if(begin(m_observers), end(m_observers), [observer](const auto *element) {
		return (element == observer);
	}), end(m_observers));
}

Asset_Observer::Asset_Observer(Asset * asset)
{
	asset->AddObserver(this);
}

Asset_Observer::~Asset_Observer()
{	
}