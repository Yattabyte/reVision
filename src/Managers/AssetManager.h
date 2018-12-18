#pragma once
#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "Assets\Asset.h"
#include "Utilities\MappedChar.h"
#include <chrono>
#include <deque>
#include <functional>
#include <future>
#include <shared_mutex>
#include <thread>


constexpr unsigned int ASSETMANAGER_MAX_THREADS = 8u;
using Asset_Work_Order = std::function<void(void)>;
class Engine;

/** Manages the storage and retrieval of assets. */
class AssetManager {
public:
	// (de)Constructors
	/** Destroy the asset manager. */
	~AssetManager() = default;
	/** Destroy the asset manager. */
	AssetManager(Engine * engine);

	
	// Public Methods
	/** Checks if an asset already exists with the given filename, fetching if true.
	@param	assetType			the name of the asset type to search for.
	@param	filename			the relative filename (within the project directory) of the asset to search for.
	@return						the asset, if found, or blank otherwise. */
	Shared_Asset shareAsset(const char * assetType, const std::string & filename);
	/** Submits a new asset to the asset manager for initialization and storage.
	@param	assetType			the name of the asset type to store it under.
	@param	asset				the asset to store.
	@param	ini					asset initialization function.
	@param	threaded			flag to create in a separate thread	*/
	void submitNewAsset(const char * assetType, Shared_Asset asset, const Asset_Work_Order && ini, const bool & threaded);
	/** Pop's the first work order and completes it. */
	void beginWorkOrder();
	/** Forwards an asset-is-finalized notification request, which will be activated from the main thread. */
	void submitNotifyee(const std::pair<std::shared_ptr<bool>, std::function<void()>> & callBack);
	/* From the main thread, calls all notification calls (for completed asset loading). */
	void notifyObservers();
	/** Returns whether or not this manager is ready to use.
	@return					true if all work is finished, false otherwise. */
	const bool readyToUse();
	/** Returns whether or not any changes have occured to this manager since the last check
	@return					true if any changes occured, false otherwise */
	const bool hasChanged();


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	std::shared_mutex m_Mutex_Assets;
	VectorMap<Shared_Asset> m_AssetMap;
	std::shared_mutex m_Mutex_Workorders;
	std::deque<Asset_Work_Order> m_Workorders;
	std::shared_mutex m_mutexNofications;
	std::vector<std::pair<std::shared_ptr<bool>, std::function<void()>>> m_notifyees;
	bool m_changed = true;
};

#endif // ASSETMANAGER_H
