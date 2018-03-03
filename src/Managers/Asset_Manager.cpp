#include "Managers\Asset_Manager.h"


Asset_Manager::Asset_Manager()
{
}

void Asset_Manager::_startup()
{
	m_Workers.reserve(4);
	for (unsigned int x = 0; x < 4; ++x) {
		m_Workers.push_back(std::make_shared<Assets_Worker>());
		auto &worker = m_Workers[x];
		unique_lock<shared_mutex> writeGuard(worker->m_mutex);
		worker->m_thread = new thread(&Asset_Manager::_threaded_func, this, worker);
	}
}

void Asset_Manager::_shutdown()
{
}

void Asset_Manager::Add_Work_Order(Work_Order * order, const bool & finalizeOnly) {
	auto &manager = Get();
	unique_lock<shared_mutex> manager_writeGuard(manager.m_Mutex_Workorders);
	if (!finalizeOnly)
		manager.m_WorkOrders_to_initialize.push_back(order);
	else
		manager.m_WorkOrders_to_finalize.push_back(order);
}

void Asset_Manager::_threaded_func(shared_ptr<Assets_Worker> & worker)
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
			workOrder->initializeOrder();

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
		workOrder->finalizeOrder();
		delete workOrder;
	}
}

shared_mutex & Asset_Manager::Get_Mutex_Assets()
{ 
	return Get().m_Mutex_Assets; 
}

VectorMap<Shared_Asset> & Asset_Manager::Get_Assets_Map()
{
	return Get().m_AssetMap;
}

void Asset_Manager::Queue_Notification(const vector<function<void()>> & callbacks)
{
	auto &manager = Get();
	unique_lock<shared_mutex> guard(manager.m_Mutex_Assets);
	manager.m_notifyees.insert(end(manager.m_notifyees), begin(callbacks), end(callbacks)); // dump new list of observers onto end of list	
}

void Asset_Manager::Notify_Observers()
{
	auto &manager = Get();
	unique_lock<shared_mutex> guard(manager.m_Mutex_Assets);

	for each (const auto & notifyee in manager.m_notifyees)
		notifyee();
	manager.m_notifyees.clear();
}
