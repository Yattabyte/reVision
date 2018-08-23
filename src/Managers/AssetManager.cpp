#include "Managers\AssetManager.h"

 
AssetManager::~AssetManager()
{	
	for (int x = 0; x < m_Workers.size(); ++x) {
		m_Workers[x].second.set_value();
		if (m_Workers[x].first.joinable())
			m_Workers[x].first.join();
	}
	m_Workers.clear();
}

AssetManager::AssetManager(Engine * engine) : m_engine(engine)
{
	// Initialize worker threads
	const unsigned int maxThreads = std::min(4u, std::min(ASSETMANAGER_MAX_THREADS, std::thread::hardware_concurrency()));
	for (unsigned int x = 0; x < maxThreads; ++x) {
		std::promise<void> exitSignal;
		std::future<void> exitObject = exitSignal.get_future();
		std::thread workerThread(&AssetManager::initializeOrders, this, std::move(exitObject));
		workerThread.detach();
		m_Workers.push_back(std::move(std::make_pair(std::move(workerThread), std::move(exitSignal))));
	}
}

void AssetManager::initializeOrders(std::future<void> exitObject)
{
	// Check if worker should shutdown
	while (exitObject.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
		// Start reading work orders
		std::unique_lock<std::shared_mutex> worker_writeGuard(m_Mutex_Workorders);
		if (m_Work_toStart.size()) {
			// Remove front of queue
			Asset_Work_Order * workOrder = m_Work_toStart.front();
			m_Work_toStart.pop_front();
			worker_writeGuard.unlock();
			worker_writeGuard.release();

			// Initialize asset
			workOrder->start();

			// Add to finalization queue
			std::unique_lock<std::shared_mutex> new_manager_writeGuard(m_Mutex_Workorders);
			m_Work_toFinish.push_back(workOrder);
		}
	}
}

void AssetManager::finalizeOrders() 
{
	std::unique_lock<std::shared_mutex> worker_writeGuard(m_Mutex_Workorders);

	// Quit early
	if (!m_Work_toFinish.size())
		return;

	// Remove front of queue
	Asset_Work_Order * workOrder = m_Work_toFinish.front();
	m_Work_toFinish.pop_front();
	worker_writeGuard.unlock();
	worker_writeGuard.release();

	// Finalize asset
	workOrder->finish();

	// Delete work order
	delete workOrder;	
}

void AssetManager::submitNotifyee(void * pointerID, const std::function<void()> & callBack)
{
	m_notifyees.push_back(std::make_pair(pointerID,callBack));
}

void AssetManager::removeNotifyee(void * pointerID)
{
	m_notifyees.erase(std::remove_if(begin(m_notifyees), end(m_notifyees), [pointerID](const std::pair<void*, std::function<void()>> & notifyee) {
		return (notifyee.first == pointerID);
	}), end(m_notifyees));
}

void AssetManager::notifyObservers()
{
	for each (const auto & notifyee in m_notifyees)
		notifyee.second();
	m_notifyees.clear();
}

const bool AssetManager::finishedWork()
{
	std::shared_lock<std::shared_mutex> readGuard(m_Mutex_Workorders);
	return !bool(m_Work_toStart.size() + m_Work_toFinish.size());
}
