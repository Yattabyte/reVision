#include "Managers\Asset_Manager.h"



Asset_Manager::Asset_Manager()
{
}

void Asset_Manager::_startup()
{
	m_Workers.reserve(3);
	for (unsigned int x = 0; x < 3; ++x) {
		m_Workers.push_back(std::make_shared<Assets_Worker>());
		auto &worker = m_Workers[x];
		unique_lock<shared_mutex> writeGuard(worker->m_mutex);
		worker->m_thread = new thread(&Asset_Manager::_threaded_func, this, worker);
	}
}

void Asset_Manager::_shutdown()
{
}

void Asset_Manager::AddWorkOrder(Work_Order * order, const bool & finalizeOnly) {
	auto &manager = Get();
	unique_lock<shared_mutex> manager_writeGuard(manager.m_Mutex_Workorders);
	if (!finalizeOnly)
		manager.m_WorkOrders_to_initialize.push_back(order);
	else
		manager.m_WorkOrders_to_finalize.push_back(order);
}

void Asset_Manager::_threaded_func(shared_ptr<Assets_Worker> &worker)
{
	bool stay_alive = true;
	while (stay_alive) {		
		// Start reading from Asset Manager
		unique_lock<shared_mutex> manager_writeGuard(m_Mutex_Workorders);
		Work_Order *workOrder = nullptr;
		if (m_WorkOrders_to_initialize.size()) {
			workOrder = m_WorkOrders_to_initialize.front();
			m_WorkOrders_to_initialize.pop_front();
		}
		manager_writeGuard.unlock();
		manager_writeGuard.release();
		if (workOrder != nullptr) {
			workOrder->Initialize_Order();

			unique_lock<shared_mutex> new_manager_writeGuard(m_Mutex_Workorders);
			m_WorkOrders_to_finalize.push_back(workOrder);
		}
		
		// Check if worker should shutdonw
		shared_lock<shared_mutex> worker_readGuard(worker->m_mutex);
		stay_alive = worker->m_alive;
	}
}

void Asset_Manager::Finalize_Orders()
{
	auto &manager = Get();
	unique_lock<shared_mutex> manager_writeGuard(manager.m_Mutex_Workorders);
	if (manager.m_WorkOrders_to_finalize.size()) {
		auto *workOrder = manager.m_WorkOrders_to_finalize.front();
		manager.m_WorkOrders_to_finalize.pop_front();
		// Unlock guard ASAP so other threads can get to work
		manager_writeGuard.unlock();
		manager_writeGuard.release();
		workOrder->Finalize_Order();
		delete workOrder;
	}
}

shared_mutex & Asset_Manager::GetMutex_Assets() 
{ 
	return Get().m_Mutex_Assets; 
}

map<int, vector<Shared_Asset>>& Asset_Manager::GetAssets_Map()
{
	return Get().m_AssetMap;
}

vector<Shared_Asset>& Asset_Manager::GetAssets_List(const int & asset_type)
{
	// Returns the vector of assets in the asset map at the spot of asset_type.
	// First tries to insert a vector in the map with the key of asset_type.
	// Map disallows duplicates, so this vector won't insert if the asset_type already exists.
	auto &manager = Get();
	unique_lock<shared_mutex> guard(manager.m_Mutex_Assets);
	manager.m_AssetMap.insert(pair<int, vector<Shared_Asset>>(asset_type, vector<Shared_Asset>()));
	return manager.m_AssetMap[asset_type];
}

void Asset_Manager::Queue_Notification(const vector<Asset_Observer*>& observers)
{
	auto &manager = Get();
	unique_lock<shared_mutex> guard(manager.m_Mutex_Assets);
	manager.m_observers.insert(end(manager.m_observers), begin(observers), end(observers)); // dump new list of observers onto end of list
}

void Asset_Manager::Noitfy_Observers()
{
	auto &manager = Get();
	unique_lock<shared_mutex> guard(manager.m_Mutex_Assets);
	for each (auto *observer in manager.m_observers) 
		observer->Notify_Finalized();
	
	manager.m_observers.clear();
}
