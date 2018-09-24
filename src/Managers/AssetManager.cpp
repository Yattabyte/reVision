#include "Managers\AssetManager.h"

 
AssetManager::AssetManager(Engine * engine) : m_engine(engine) {}

void AssetManager::submitNewWorkOrder(const Asset_Work_Order && ini, const bool & threaded) {
	if (threaded) {
		std::unique_lock<std::shared_mutex> worker_writeGuard(m_Mutex_Workorders);
		m_Workorders.push_back(std::move(ini));
	}
	else
		ini();
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

void AssetManager::submitNotifyee(void * pointerID, const std::function<void()> & callBack)
{
	std::lock_guard writeGuard(m_mutexNofications);
	m_notifyees.push_back(std::make_pair(pointerID,callBack));
}

void AssetManager::removeNotifyee(void * pointerID)
{
	std::lock_guard writeGuard(m_mutexNofications);
	m_notifyees.erase(std::remove_if(begin(m_notifyees), end(m_notifyees), [pointerID](const std::pair<void*, std::function<void()>> & notifyee) {
		return (notifyee.first == pointerID);
	}), end(m_notifyees));
}

void AssetManager::notifyObservers()
{
	std::vector<std::pair<void*, std::function<void()>>> copyNotifyees;
	{
		std::lock_guard writeGuard(m_mutexNofications);
		copyNotifyees = m_notifyees;
		m_notifyees.clear();
	}
	for each (const auto & notifyee in copyNotifyees)
		notifyee.second();
}

const bool AssetManager::finishedWork()
{
	std::shared_lock<std::shared_mutex> readGuard(m_Mutex_Workorders);
	return !bool(m_Workorders.size() + m_Workorders.size());
}
