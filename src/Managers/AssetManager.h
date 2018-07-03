#pragma once
#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "Assets\Asset.h"
#include "Utilities\MappedChar.h"
#include <deque>
#include <functional>
#include <shared_mutex>
#include <thread>


class Engine;

/** Base class that represents a function, but does nothing else */
struct FuncBase {};

/** This class represents a specific function with a specific signature (templated) */
template <typename... Args>
struct FuncHolder : public FuncBase {
	FuncHolder(const function<void(Args...)> & f) : m_function(f) {	}

	function<void(Args...)> m_function;
};

struct WorkOrder_Base {
	virtual void start() = 0;
	virtual void finish() = 0;
};

template <typename... Args>
struct WorkOrder_Specialized : public WorkOrder_Base {
	WorkOrder_Specialized(const function<void(Args...)> & i, const function<void(Args...)> & f) : m_ini(i), m_fin(f) {}
	virtual void start() { m_ini(); }
	virtual void finish() { m_fin(); }

	function<void(Args...)> m_ini, m_fin;
};

/** 
 * Manages the storage and retrieval of assets.
 **/
class AssetManager
{
public:
	// (de)Constructors
	/** Destroy the asset manager. */
	~AssetManager();
	/** Destroy the asset manager. */
	AssetManager(Engine * engine);


	// Public Methods
	/** Creates an asset or uses a cached copy if it has already been created.
	 * @param	sharedAsset		the cointainer to place the asset
	 * @param	args			the rest of the arguments to be used for initialization
	 */
	template <typename SharedAsset, typename... Args>
	void create(SharedAsset & sharedAsset, Args&&... ax) {
		// Get the asset's name from the template, and forward the engine pointer, asset container, and extra arguments to the creator function
		forwardMapArguments(typeid(SharedAsset).name(), m_engine, sharedAsset, forward<Args>(ax)...);
	}
	/** Queries if an asset already exists with the given filename, fetching if true.
	 * @brief				Searches for and updates the supplied container with the desired asset if it already exists.
	 * @param	<Asset_T>	the type of asset to query for
	 * @param	user		the shared_ptr container to load the asset into if the query is successful
	 * @param	filename	the relative filename (within the project directory) of the asset to search for
	 * @return				true if it was successful in finding the asset, false otherwise */
	template <typename Asset_T>
	bool queryExistingAsset(shared_ptr<Asset_T> & user, const string & filename) {
		shared_lock<shared_mutex> guard(m_Mutex_Assets);
		for each (auto &asset in m_AssetMap[typeid(Asset_T).name()]) {
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
	/** A template for creating assets and forwarding their arguments, also adds to the map. 
	 * @param	userAsset	the asset container
	 * @param	ax			the constructor arguments */
	template <typename Asset_T, typename... Args>
	void createNewAsset(shared_ptr<Asset_T> & userAsset, Args&&... ax) {
		userAsset = shared_ptr<Asset_T>(new Asset_T(forward<Args>(ax)...));
		m_AssetMap[typeid(Asset_T).name()].push_back(userAsset);
	}
	/** Submits an asset for physical creation, and optionally whether to thread it or not. To be called by asset creation functions.
	 * @param	userAsset	the asset container
	 * @param	threaded	flag to create in a separate thread
	 * @param	ini			lambda initialization function
	 * @param	fin			lambda finalization function
	 * @param	ax			args to forward to the asset constructor
	 */
	template <typename Asset_T, typename Init_Callback, typename Fin_Callback, typename... Args>
	void submitNewAsset(shared_ptr<Asset_T> & userAsset, const bool & threaded, Init_Callback && ini, Fin_Callback && fin, Args&&...ax) {
		createNewAsset(userAsset, forward<Args>(ax)...);
		submitNewWorkOrder(userAsset, threaded, ini, fin);
	}
	/** Submits an asset for physical creation, and optionally whether to thread it or not. To be called by asset creation functions.
	 * @param	userAsset	the asset container
	 * @param	threaded	flag to create in a separate thread
	 * @param	ini			lambda initialization function
	 * @param	fin			lambda finalization function
	 * @param	ax			args to forward to the asset constructor
	 */
	template <typename Asset_T, typename Init_Callback, typename Fin_Callback>
	void submitNewWorkOrder(shared_ptr<Asset_T> & userAsset, const bool & threaded, Init_Callback && ini, Fin_Callback && fin) {
		if (threaded) {
			unique_lock<shared_mutex> worker_writeGuard(m_Mutex_Workorders);
			m_Work_toStart.push_back(new WorkOrder_Specialized<void>(ini, fin));
		}
		else {
			ini();
			fin();
		}
	}
	/** Finalize any initialized orders. */
	void finalizeOrders();
	/** For assets that have just finalized, takes callback submissions. */
	void submitNotifyee(const function<void()> & callBack);
	/* From the main thread, calls all notification calls (for completed asset loading). */
	void notifyObservers();


private:
	// Private Structures
	struct Asset_Worker {
		Asset_Worker() {
			m_alive = true;
			m_thread = nullptr;
		}
		~Asset_Worker() {
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
		

		bool m_alive;
		thread *m_thread;
		shared_mutex m_mutex;
	};


	// Private Methods
	/** A template for registrating new asset creators. 
	 * @param	type	the name to use
	 * @param	f		the creator function to use */
	template <typename... Args>
	void registerAssetCreator(const char * type, const function<void(Args...)> & f) {
		m_CreatorMap[type] = new FuncHolder<Args...>(f);
	}
	/* A template to allow concatenating several parameters into one, for forwarding purposes. 
	 * @param	type	the name to use
	 * @param	ax		the aruments to forward */
	template <typename... Args>
	void forwardMapArguments(const char * type, Args&&... ax) {
		((FuncHolder<Args...>*)(m_CreatorMap[type]))->m_function(forward<Args>(ax)...);
	}
	void initializeOrders(shared_ptr<Asset_Worker> & worker);


	// Private Attributes
	Engine * m_engine;
	MappedChar<FuncBase *> m_CreatorMap;
	shared_mutex m_Mutex_Assets;
	VectorMap<Shared_Asset> m_AssetMap;
	shared_mutex m_Mutex_Workorders;
	deque<WorkOrder_Base*> m_Work_toStart, m_Work_toFinish;
	vector<shared_ptr<Asset_Worker>> m_Workers;	
	vector<function<void()>> m_notifyees;
};

#endif // ASSETMANAGER_H
