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
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Assets\Asset.h"
#include "Utilities\MappedChar.h"
#include <shared_mutex>
#include <thread>
#include <string>
#include <deque>
#include <vector>
#include <cstdarg>
#include <utility>
#include <typeinfo.h>

using namespace std;


/**
 * Manages the storage and retrieval of assets.\n
 * Features:
 *		- Is a singleton, shares its data across the entire application with ease.
 *		- Can store and retrieve assets from different threads.
 *		- Supports multi-threading
 **/
class DT_ENGINE_API Asset_Manager {
public:
	// Public Methods
	/** Singleton GET method.
	 * @return	static Asset_Manager instance */
	static Asset_Manager & Get() {
		static Asset_Manager instance;
		return instance;
	}
	/** Start up and initialize the asset manager. */
	static void Start_Up() { Get()._startup(); }
	/** Shut down and flush out the asset manager. */
	static void Shut_Down() { Get()._shutdown(); }
	/** Submit a new work order request.
	 * @brief					Uses multiple worker-threads. Calls order initialize function.
	 * @param	order			the work order to fulfill
	 * @param	onlyFinalize	if true, the work order should only be finalized (skip initialization) 
	 * @note					If the asset doesn't support multi-threading, initialize it first and set onlyFinalize to true! */
	static void Add_Work_Order(Work_Order * order, const bool & onlyFinalize = false);
	/** Finalizes work orders that have finished initializing.
	 * @brief	Occurs in main thread. Calls order finalize function. Acts as a synchronization point.
	 * @note	Immediately after finalizing, assets will call their notify function alerting their observers. */
	static void Finalize_Orders();
	/** Retrieves the asset map mutex
	 * @return	the mutex used for accessing the asset map */
	static shared_mutex & Get_Mutex_Assets();
	/** Retrieves the map containing all the assets 
	 * @return	the asset map */
	static VectorMap<Shared_Asset> & Get_Assets_Map();
	/** Retrieves the vector of assets within the map that match the supplied type <Asset_T>.
	 * @brief				A helper function bypassing the need to first retrieve the map when only 1 type of asset is needed
	 * @param	<Asset_T>	Any type of asset that extends the base class Asset
	 * @param	asset_type	the category of asset to retrieve from the map
	 * @return				the vector belonging to the category of assets chosen
	 * @note				This will create the category if it didn't already exist. Guaranteed to return a vector.*/
	template <typename Asset_t>
	static vector<Shared_Asset> & Get_Assets_List() {
		// Returns the vector of assets in the asset map at the spot of asset_type.
		// First tries to insert a vector in the map with the key of asset_type.
		// Map disallows duplicates, so this vector won't insert if the asset_type already exists.
		auto &manager = Get();
		const char * asset_type = typeid(Asset_t).name();
		unique_lock<shared_mutex> guard(manager.m_Mutex_Assets);
		return manager.m_AssetMap[asset_type];
	}
	/** Creates a new asset of the supplied type and arguments
	 * @param	<Asset_T>	The Asset class type to create
	 * @param	user		the shared_ptr container for the asset
	 * @param	_Ax			the arguments to send to the assets constructor
	 * @note				The asset will be zero-initialized, and requires submission as a work order. Safe to share. */
	template <typename Asset_T, typename... _Args>
	static void Create_New_Asset(shared_ptr<Asset_T> & user, _Args&&... _Ax) {
		user = shared_ptr<Asset_T>(new Asset_T(forward<_Args>(_Ax)...)); // new asset of type asset_t, ARGS held in _Ax		
		(Asset_Manager::Get_Assets_List<Asset_T>()).push_back(user); // add vector in asset map
	}
	/** Creates and submits a new asset of the supplied type and arguments. Generates the work order too.
	 * @brief					Creates the asset and then generates the work order. 
	 * @param	<Asset_T>		The Asset class type to create
	 * @param	user			the shared_ptr container for the asset
	 * @param	threaded		if true, submits the work order onto a separate thread
	 * @param	fullDirectory	absolute path to the file on disk for this asset
	 * @param	_Ax				the arguments to send to the asset's constructor */
	template <typename Asset_T, typename Workorder_T, typename... _Args>
	static void Submit_New_Asset(shared_ptr<Asset_T> & user, const bool & threaded, const string & fullDirectory, _Args&&... _Ax) {
		Create_New_Asset<Asset_T>(user, _Ax...);
		if (threaded)
			Asset_Manager::Add_Work_Order(new Workorder_T(user, fullDirectory));
		else {
			Workorder_T work_order(user, fullDirectory);
			work_order.initializeOrder();
			work_order.finalizeOrder();
		}
	}
	/** Queries if an asset already exists with the given filename, fetching if true
	 * @brief				Searches for and updates the supplied container with the desired asset if it already exists.
	 * @param	<Asset_T>	the type of asset to query for
	 * @param	user		the shared_ptr container to load the asset into if the query is successful
	 * @param	filename	the relative filename (within the project directory) of the asset to search for
	 * @return				true if it was successful in finding the asset, false otherwise */
	template <typename Asset_T>
	static bool Query_Existing_Asset(shared_ptr<Asset_T> & user, const string & filename) {
		auto &asset_list = (Asset_Manager::Get_Assets_List<Asset_T>());
		shared_lock<shared_mutex> guard(Asset_Manager::Get_Mutex_Assets());
		for each (auto &asset in asset_list) {
			shared_lock<shared_mutex> asset_guard(asset->m_mutex);
			// No need to cast it, filename is a member variable across assets
			if (asset->getFileName() == filename) {
				asset_guard.unlock();
				asset_guard.release();
				user = dynamic_pointer_cast<Asset_T>(asset);

				// Can't guarantee that the asset isn't already being worked on, so no finalization here if threaded
				return true;
			}
		}
		return false;
	}
	/** Appends a list of observers to notify later that their asset has achieved a desired state
	 * @brief				when the main thread is ready, it will notify all the assets that have queued up.
	 * @param	callbacks	the list of observer functions to notify */
	static void Queue_Notification(const vector<function<void()>> & callbacks);
	/** Notifies all the queued up observers that their assets have finished finalization.
	 * @note				is called from the main thread only to ensure proper synchronization. */
	static void Notify_Observers();


private:
	/** Nested Asset Worker
	 * @brief is kind of pointless, but this can be made much better
	 * @todo make the material buffer have add/remove functions, and control sending its data to the GPU. Also store removed spots here.
	 **/
	static class Assets_Worker
	{
	public:
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

	// (de)Constructors all private
	~Asset_Manager() {};
	Asset_Manager();
	Asset_Manager(Asset_Manager const&) = delete;
	void operator=(Asset_Manager const&) = delete;


	// Public Methods
	void _startup();
	void _shutdown();
	void _threaded_func(shared_ptr<Assets_Worker> &worker);


	// Public Attributes
	bool m_Initialized;
	vector<shared_ptr<Assets_Worker>> m_Workers;
	shared_mutex m_Mutex_Workorders;
	deque<Work_Order*> m_WorkOrders_to_initialize;
	deque<Work_Order*> m_WorkOrders_to_finalize;
	shared_mutex m_Mutex_Assets;
	VectorMap<Shared_Asset> m_AssetMap;	
	shared_mutex m_Mutex_Callbacks;
	vector<function<void()>> m_notifyees;
};

#endif // ASSET_MANAGER
