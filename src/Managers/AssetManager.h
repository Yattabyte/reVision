#pragma once
#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include "Assets/Asset.h"
#include "Utilities/MappedChar.h"
#include <deque>
#include <shared_mutex>


constexpr unsigned int ASSETMANAGER_MAX_THREADS = 8u;
using Asset_Work_Order = std::function<void(void)>;

/** Manages the storage and retrieval of assets. */
class AssetManager {
public:
	// Public (De)Constructors
	/** Destroy the asset manager. */
	inline ~AssetManager() = default;
	/** Create the asset manager. */
	inline AssetManager() = default;


	// Public Methods
	/** Checks if an asset already exists with the given filename, fetching if true.
	@param	assetType			the name of the asset type to search for.
	@param	filename			the relative filename (within the project directory) of the asset to search for.
	@param	constructor			a construction method, for creating the asset should it be needed.
	@param	threaded			flag to create in a separate thread.
	@return						the asset, if found, or blank otherwise. */
	[[nodiscard]] Shared_Asset shareAsset(const char* assetType, const std::string& filename, const std::function<Shared_Asset(void)>& constructor, const bool& threaded) noexcept;
	/** Pop's the first work order and completes it. */
	void beginWorkOrder() noexcept;
	/** Forwards an asset-is-finalized notification request, which will be activated from the main thread. */
	void submitNotifyee(const std::pair<std::shared_ptr<bool>, std::function<void()>>& callBack) noexcept;
	/** From the main thread, calls all notification calls (for completed asset loading). */
	void notifyObservers() noexcept;
	/** Retrieves whether or not this manager is ready to use.
	@return						true if all work is finished, false otherwise. */
	bool readyToUse() noexcept;
	/** Retrieves whether or not any changes have occurred to this manager since the last check.
	@return						true if any changes occurred, false otherwise. */
	bool hasChanged() noexcept;


private:
	// Private but deleted
	/** Disallow manager move constructor. */
	inline AssetManager(AssetManager&&) noexcept = delete;
	/** Disallow manager copy constructor. */
	inline AssetManager(const AssetManager&) noexcept = delete;
	/** Disallow manager move assignment. */
	inline const AssetManager& operator =(AssetManager&&) noexcept = delete;
	/** Disallow manager copy assignment. */
	inline const AssetManager& operator =(const AssetManager&) noexcept = delete;


	// Private Attributes
	std::shared_mutex m_Mutex_Assets;
	VectorMap<Shared_Asset> m_AssetMap;
	std::shared_mutex m_Mutex_Workorders;
	std::deque<Asset_Work_Order> m_Workorders;
	std::shared_mutex m_mutexNofications;
	std::vector<std::pair<std::shared_ptr<bool>, std::function<void()>>> m_notifyees;
	bool m_changed = true;
};

#endif // ASSETMANAGER_H