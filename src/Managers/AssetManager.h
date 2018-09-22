#pragma once
#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "Assets\Asset.h"
#include "Utilities\MappedChar.h"
#include <deque>
#include <functional>
#include <future>
#include <shared_mutex>
#include <thread>


constexpr unsigned int ASSETMANAGER_MAX_THREADS = 8u;
class Engine;

/** Base class that represents a function, but does nothing else */
struct FuncBase {};

/** This class represents a specific function with a specific signature (templated) */
template <typename... Args>
struct FuncHolder : public FuncBase {
	FuncHolder(const std::function<void(Args...)> & f) : m_function(f) {	}

	std::function<void(Args...)> m_function;
};

/** Represents an asset work order that gets started and finished at separate times. */
struct Asset_Work_Order  {
	Asset_Work_Order(const std::function<void(void)> & i, const std::function<void(void)> & f) : m_ini(i), m_fin(f) {}
	void start() { m_ini(); }
	void finish() { m_fin(); }

	std::function<void(void)> m_ini, m_fin;
};

/** Manages the storage and retrieval of assets. */
class AssetManager {
public:
	// (de)Constructors
	/** Destroy the asset manager. */
	~AssetManager();
	/** Destroy the asset manager. */
	AssetManager(Engine * engine);

	
	// Public Methods
	/** Queries if an asset already exists with the given filename, fetching if true.
	@brief				Searches for and updates the supplied container with the desired asset if it already exists.
	@param	<Asset_T>	the type of asset to query for
	@param	filename	the relative filename (within the project directory) of the asset to search for
	@return				the asset, if found, or null if not */
	template <typename Asset_T>
	std::shared_ptr<Asset_T> queryExistingAsset(const std::string & filename) {
		std::shared_lock<std::shared_mutex> read_guard(m_Mutex_Assets);
		for each (auto &asset in m_AssetMap[typeid(Asset_T).name()]) 
			if (asset->getFileName() == filename) 
				return std::dynamic_pointer_cast<Asset_T>(asset);				
		return std::shared_ptr<Asset_T>();
	}
	/** A template for creating assets and forwarding their arguments, also adds to the map. 	
	@param	<Asset_T>	the type of asset to create
	@param	ax			the constructor arguments */
	template <typename Asset_T, typename... Args>
	std::shared_ptr<Asset_T> createNewAsset(Args&&... ax) {
		std::shared_ptr<Asset_T> userAsset = std::shared_ptr<Asset_T>(new Asset_T(std::forward<Args>(ax)...));
		std::unique_lock<std::shared_mutex> write_guard(m_Mutex_Assets);
		m_AssetMap[typeid(Asset_T).name()].push_back(userAsset);
		return userAsset;
	}
	/** Submits an asset for physical creation, and optionally whether to thread it or not. To be called by asset creation functions.
	@param	<Asset_T>	the type of asset used
	@param	userAsset	the asset container
	@param	threaded	flag to create in a separate thread
	@param	ini			lambda initialization function
	@param	fin			lambda finalization function */
	template <typename Asset_T, typename Init_Callback, typename Fin_Callback>
	void submitNewWorkOrder(std::shared_ptr<Asset_T> & userAsset, const bool & threaded, Init_Callback && ini, Fin_Callback && fin) {
		if (threaded) {
			std::unique_lock<std::shared_mutex> worker_writeGuard(m_Mutex_Workorders);
			m_Work_toStart.push_back(new Asset_Work_Order(ini, fin));
		}
		else {
			ini();
			fin();
		}
	}
	/** Finalize any initialized orders. */
	void finalizeOrders();
	/** For assets that have just finalized, takes callback submissions. */
	void submitNotifyee(void * pointerID, const std::function<void()> & callBack);
	/** Remove a notifyee from the pool. */
	void removeNotifyee(void * pointerID);
	/* From the main thread, calls all notification calls (for completed asset loading). */
	void notifyObservers();
	/** Returns whether or not this manager has work left.
	 * @return	true if all work is finished, false otherwise. */
	const bool finishedWork();


private:
	// Private Methods
	/** Initializes any waiting orders.
	@param	exitObject	object signaling when to close the thread */
	void initializeOrders(std::future<void> exitObject);


	// Private Attributes
	Engine * m_engine;
	std::shared_mutex m_Mutex_Assets;
	VectorMap<Shared_Asset> m_AssetMap;
	std::shared_mutex m_Mutex_Workorders;
	std::deque<Asset_Work_Order*> m_Work_toStart, m_Work_toFinish;
	std::shared_mutex m_workerNotificationMutex;
	std::vector<std::pair<std::thread, std::promise<void>>> m_Workers;
	std::shared_mutex m_mutexNofications;
	std::vector<std::pair<void*, std::function<void()>>> m_notifyees;
};

#endif // ASSETMANAGER_H
