#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include <algorithm>

/* -----ASSET TYPE----- */
#define ASSET_TYPE -1

Asset::~Asset()
{
}

Asset::Asset()
{	
	finalized = false;
}

int Asset::GetAssetType() 
{
	return ASSET_TYPE;
}

bool Asset::ExistsYet() 
{ 
	return finalized;
}

void Asset::Finalize()
{
	if (!finalized) {
		finalized = true;
		Asset_Manager::Queue_Notification(m_Observers); // Notify later, guaranteed to be done during rendering loop
	}
}

void Asset::AddObserver(Asset_Observer * observer)
{
	unique_lock<shared_mutex> write_guard(m_mutex);
	m_Observers.push_back(observer);
}

void Asset::RemoveObserver(Asset_Observer * observer)
{
	m_Observers.erase(std::remove_if(begin(m_Observers), end(m_Observers), [observer](const auto *element) {
		return (element == observer);
	}), end(m_Observers));
}

Asset_Observer::Asset_Observer(Asset * asset)
{
	asset->AddObserver(this);
}

Asset_Observer::~Asset_Observer()
{	
}