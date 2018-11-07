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
	/** Returns an asset matching the parameters specified. If none is found, creates a new one.
	@param	<Asset_T>	the asset class type to create
	@param	filename	the asset's file name
	@param	directory	the asset category's directory (eg \\Shaders\\), can be empty
	@param	extension	the asset category's file extension (eg .shader), can be empty
	@param	initFunc	the asset's initialization function
	@param	engine		the engine pointer
	@param	threaded	boolean for whether to create the asset in a separate thread or not
	@param	args		any constructor arguments to use when creating a new asset */
	template <typename Asset_T, typename ...Args>
	inline std::shared_ptr<Asset_T> createAsset(
		const std::string & filename, 
		const std::string & directory, 
		const std::string & extension,
		void (Asset_T::*initFunc)(Engine*,const std::string &),
		Engine * engine,
		const bool & threaded,
		Args && ...args
	) {
		// The standardized name of this asset type, for lookup purposes.
		const auto assetTypeName = typeid(Asset_T).name();

		// Find out if the asset already exists
		std::unique_lock<std::shared_mutex> write_guard(m_Mutex_Assets);
		auto userAsset = std::dynamic_pointer_cast<Asset_T>(queryExistingAsset(assetTypeName, filename, threaded));
		if (userAsset) // Asset exists
			return userAsset;
		
		// Asset doesn't exist, make it
		userAsset = std::make_shared<Asset_T>(filename, std::forward<Args>(args)...);
		m_AssetMap[assetTypeName].push_back(userAsset);
		write_guard.unlock();
		write_guard.release();

		// Submit the work order
		const std::string relativePath(directory + filename + extension);
		submitNewWorkOrder(std::move(std::bind(initFunc, userAsset.get(), engine, relativePath)), threaded);
		return userAsset;
	}	
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
	// Private Methods
	/** Queries if an asset already exists with the given filename, fetching if true.
	@brief						Searches for and updates the supplied container with the desired asset if it already exists.
	@param	assetType			the name of the asset type to search for.
	@param	filename			the relative filename (within the project directory) of the asset to search for.
	@param	dontForceFinalize	if the asset hasn't been finalized, whether or not to wait for it to finalize before returning.
	@return						the asset, if found, or null if not. */
	Shared_Asset queryExistingAsset(const char * assetType, const std::string & filename, const bool & dontForceFinalize = false);
	/** Submits an asset for physical creation, and optionally whether to thread it or not.
	@param	ini			asset initialization function
	@param	threaded	flag to create in a separate thread	*/
	void submitNewWorkOrder(const Asset_Work_Order && ini, const bool & threaded);


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
