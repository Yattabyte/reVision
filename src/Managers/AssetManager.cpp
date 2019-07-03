#include "Managers/AssetManager.h"


Shared_Asset AssetManager::shareAsset(const char * assetType, const std::string & filename, const std::function<Shared_Asset(void)> & constructor, const bool & threaded)
{
	// Find out if the asset already exists
	std::shared_lock<std::shared_mutex> asset_read_guard(m_Mutex_Assets);
	for each (const Shared_Asset asset in m_AssetMap[assetType])
		if (asset->getFileName() == filename) {
			asset_read_guard.unlock();
			asset_read_guard.release();
			// Check if we need to wait for initialization
			if (!threaded)
				// Stay here until asset finalizes
				while (!asset->existsYet())
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
			return asset;
		}
	asset_read_guard.unlock();
	asset_read_guard.release();

	// Create the asset
	std::unique_lock<std::shared_mutex> asset_write_guard(m_Mutex_Assets);
	const auto & asset = constructor();
	m_AssetMap[assetType].push_back(asset);
	asset_write_guard.unlock();
	asset_write_guard.release();

	// Initialize now or later, depending if we are threading this order or not
	const auto & initFunction = std::bind(&Asset::initialize, asset);
	if (threaded) {
		std::unique_lock<std::shared_mutex> worker_write_guard(m_Mutex_Workorders);
		m_Workorders.push_back(std::move(initFunction));
	}
	else
		initFunction();
	return asset;
}

void AssetManager::beginWorkOrder()
{
	// Start reading work orders
	std::unique_lock<std::shared_mutex> writeGuard(m_Mutex_Workorders);
	if (m_Workorders.size()) {
		// Remove front of queue
		const Asset_Work_Order workOrder = m_Workorders.front();
		m_Workorders.pop_front();
		writeGuard.unlock();
		writeGuard.release();

		// Initialize asset
		workOrder();
	}
}

void AssetManager::submitNotifyee(const std::pair<std::shared_ptr<bool>, std::function<void()>> & callBack)
{
	std::unique_lock<std::shared_mutex> writeGuard(m_mutexNofications);
	m_notifyees.push_back(callBack);
	m_changed = true;
}

void AssetManager::notifyObservers()
{
	std::vector<std::pair<std::shared_ptr<bool>, std::function<void()>>> copyNotifyees;
	{
		std::unique_lock<std::shared_mutex> writeGuard(m_mutexNofications);
		copyNotifyees = m_notifyees;
		m_notifyees.clear();
	}
	for each (const auto & pair in copyNotifyees)
		if (pair.first)
			pair.second();
}

bool AssetManager::readyToUse()
{	
	{
		std::shared_lock<std::shared_mutex> readGuard(m_Mutex_Workorders);
		if (m_Workorders.size())
			return false;
	}
	{
		std::shared_lock<std::shared_mutex> readGuard(m_Mutex_Assets);
		for each (const auto & assetCategory in m_AssetMap)
			for each (const auto & asset in assetCategory.second)
				if (!asset->existsYet())
					return false;
	}
	return true;
}

bool AssetManager::hasChanged()
{
	// Changes every time assets finalize, when this manager notifies the assets' observers.
	std::shared_lock<std::shared_mutex> readGuard(m_mutexNofications);
	const bool state = m_changed;
	m_changed = false;
	return state;
}
