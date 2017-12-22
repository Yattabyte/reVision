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
#include <cstdarg>
#include <utility>

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
	static void Finalize_Orders();
	// Retrieves the asset map mutex
	static shared_mutex& GetMutex_Assets();
	// Retrieves the map of all assets
	static map<int, vector<Shared_Asset>>& GetAssets_Map();
	// Retrieves the map of all fallback assets
	static map<int, Shared_Asset>& GetFallbackAssets_Map();
	// Retrieves the vector of assets of the given type. Will create one if it doesn't exist.
	static vector<Shared_Asset>& GetAssets_List(const int &asset_type);
	// Create a new asset of the supplied type and filename 
	template <typename Asset_T, typename Workorder_T, class... _Args>
	static void CreateNewAsset(shared_ptr<Asset_T> & user, const bool & threaded, const string & fullDirectory, _Args&&... _Ax) {
		user = shared_ptr<Asset_T>(new Asset_T(forward<_Args>(_Ax)...)); // new asset of type asset_t, args held in _Ax		
		(Asset_Manager::GetAssets_List(Asset_T::GetAssetType())).push_back(user); // add vector in asset map
		if (threaded)
			Asset_Manager::AddWorkOrder(new Workorder_T(user, fullDirectory));
		else {
			Workorder_T work_order(user, fullDirectory);
			work_order.Initialize_Order();
			work_order.Finalize_Order();
		}
	}
	// Query if desired asset already exists
	template <typename Asset_T>
	static bool QueryExistingAsset(shared_ptr<Asset_T> & user, const string &filename) {
		auto &asset_list = (Asset_Manager::GetAssets_List(Asset_T::GetAssetType()));
		shared_lock<shared_mutex> guard(Asset_Manager::GetMutex_Assets());
		for each (auto &asset in asset_list) {
			shared_lock<shared_mutex> asset_guard(asset->m_mutex);
			// No need to cast it, filename is a member variable across assets
			if (asset->GetFileName() == filename) {
				asset_guard.unlock();
				asset_guard.release();
				user = dynamic_pointer_cast<Asset_T>(asset);

				// Can't guarantee that the asset isn't already being worked on, so no finalization here if threaded
				return true;
			}
		}
		return false;
	}
	// Return the default asset of the provided category
	template <typename Asset_T>
	static bool RetrieveDefaultAsset(shared_ptr<Asset_T> & user, const string &defaultText) {
		shared_lock<shared_mutex> guard(Asset_Manager::GetMutex_Assets());
		map<int, Shared_Asset> &fallback_assets = Asset_Manager::GetFallbackAssets_Map();
		const auto &type = Asset_T::GetAssetType();
		fallback_assets.insert(pair<int, Shared_Asset>(type, Shared_Asset()));
		auto &defaultAsset = fallback_assets[type];
		if ((defaultAsset.get() == nullptr)) {
			defaultAsset = shared_ptr<Asset_T>(new Asset_T(defaultText));
			user = dynamic_pointer_cast<Asset_T>(defaultAsset);
			return false;
		}
		return true;
	}
	// Queue up a list of observers to notify them that their asset has finalized
	static void Queue_Notification(const vector<Asset_Observer*> &observers);
	// Notifies the observers of assets that completed finalization
	static void Noitfy_Observers();


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
	vector<Asset_Observer*> m_observers;
};

#endif // ASSET_MANAGER
