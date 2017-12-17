/*
	Asset_Manager

	- Manages the storage and retrieval of assets.
	- Uses static variables, designed to be used across an entire application with ease.
	- Can store and retrieve assets from different threads.
	- Preliminary support for multithreading, though it is up to each asset type to support it safely.
	- Static variables kept within source file

	Note: Asset loading initialized by "load_Asset(shared_ptr<ASSET_TYPE> &asset)"
*/

#pragma once
#ifndef ASSET_MANAGER
#define ASSET_MANAGER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Assets\Asset.h"
#include <deque>
#include <map>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

using namespace std;

struct Assets_Worker {
	bool m_alive;
	thread *m_thread;
	shared_mutex m_mutex;

	Assets_Worker() {
		m_alive = true;
		m_thread = nullptr;
	}
	~Assets_Worker() {
		{
			unique_lock<shared_mutex> writeLock(m_mutex);
			m_alive = false;
		}
		shared_lock<shared_mutex> readLock(m_mutex);
		if (m_thread != nullptr) {
			if (m_thread->joinable())
				m_thread->join();
			readLock.unlock();
			readLock.release();
			unique_lock<shared_mutex> writeLock(m_mutex);
			delete m_thread;
		}

	}
};

class DT_ENGINE_API Work_Order {
public:
	// Boring ole constructor
	Work_Order() {};
	// Virtual destructor
	virtual ~Work_Order() {};
	// To be used for loading data from disk in a thread safe way
	virtual void Initialize_Order() = 0;
	// To be used for attaching to opengl state
	virtual void Finalize_Order() = 0;
};

class DT_ENGINE_API Asset_Manager {
public:
	static Asset_Manager &Get() {
		static Asset_Manager instance;
		return instance;
	}
	// Start up and initialize the asset manager
	static void Startup() { Get()._startup(); }
	// Shut down and flush out the asset manager
	static void Shutdown() { Get()._shutdown(); }
	// Add a new work order request
	static void AddWorkOrder(Work_Order* order);
	// Peforms the last stage of processing on work orders that couldn't be threaded
	// Eg: creating texture objects and making them resident on the GPU
	static void Finalize_WorkOrders_Threaded();
	// Retrieves the asset map mutex
	static shared_mutex& GetMutex_Assets();
	// Retrieves the map of all assets
	static map<int, vector<Shared_Asset>>& GetAssets_Map();
	// Retrieves the map of all fallback assets
	static map<int, Shared_Asset>& GetFallbackAssets_Map();
	// Retrieves the vector of assets of the given type. Will create one if it doesn't exist.
	static vector<Shared_Asset>& GetAssets_List(const int &asset_type);


private:
	~Asset_Manager() {};
	Asset_Manager();
	Asset_Manager(Asset_Manager const&) = delete;
	void operator=(Asset_Manager const&) = delete;

	void _startup();
	void _shutdown();
	void _threaded_func(shared_ptr<Assets_Worker> &worker);

	bool m_Initialized;
	vector<shared_ptr<Assets_Worker>> m_Workers;

	shared_mutex m_Mutex_Workorders;
	deque<Work_Order*> m_WorkOrders_to_initialize;
	deque<Work_Order*> m_WorkOrders_to_finalize;

	shared_mutex m_Mutex_Assets;
	map<int, vector<Shared_Asset>> m_AssetMap;
	map<int, Shared_Asset> m_AssetMap_Fallback;										
};



#endif // ASSET_MANAGER