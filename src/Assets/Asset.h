#pragma once
#ifndef	ASSET_H
#define	ASSET_H

#include <shared_mutex>
#include <vector>
#include <stdio.h>
#include <string>
#include <memory>
#include <map>
#include <utility>


class Asset;
typedef std::shared_ptr<Asset> Shared_Asset;

/**
 * An abstract base-class for assets.
 * @brief	Represents some form of data to be loaded from disk, such as shaders, models, levels, and sounds.
 * @note	is an abstract class instead of interface to reduce redundant code.
 * @usage	Should be created once, and its pointer passed around using shared pointers.
 **/
class Asset
{
public:
	// (de)Constructors
	/** Destroy the asset only when all references are destroyed. */
	~Asset();
	/** Create asset that uses the specified file-path. */
	Asset(const std::string & filename = "");


	// Public Methods	
	/** Gets the file name of this asset.
	 * @return				the file name belonging to this asset */
	std::string getFileName() const;
	/** Sets the file name of this asset.
	 * @param	filename	the file name to set this asset to */
	void setFileName(const std::string & filename);	
	/** Attaches a callback method to be triggered when the asset finishes loading.
	 * @param	pointerID	the pointer to the object owning the function. Used for sorting and removing the callback.
	 * @param	callback	the method to be triggered
	 * @param	<Callback>	the (auto-deduced) signature of the method */
	template <typename Callback>
	void addCallback(void * pointerID, Callback && callback) {
		std::unique_lock<std::shared_mutex> write_guard(m_mutex);
		m_callbacks[pointerID] = std::forward<Callback>(callback);
		write_guard.unlock();
		write_guard.release();
	}
	/** Removes a callback method from triggering when the asset finishes loading.
	 * @param	pointerID	the pointer to the object owning the callback to be removed */
	void removeCallback(void * pointerID);
	/** Returns whether or not this asset has completed finalizing.
	 * @note				Virtual, each asset can re-implement if they have specific finalizing criteria.
	 * @return				true if this asset has finished finalizing, false otherwise. */
	bool existsYet() const;
	/** Performs final data processing.
	 * @note	Virtual, each asset can re-implement if they have specific finalizing criteria. */
	virtual void finalize();


	// Public Attributes
	mutable std::shared_mutex m_mutex;	/** public std::mutex, to encourage safe access of asset. */


protected:
	// Protected Attributes
	bool m_finalized;
	std::string m_filename;
	std::map<void*, std::function<void()>> m_callbacks;
	friend class AssetManager;
};

/**
 * An abstract class for assets work orders.
 * @brief	Each asset should implement a specific work order specialized for their own data.
 **/
class Work_Order
{
public:
	// (de)Constructor
	/** Generic default constructor. */
	Work_Order() {};
	/** Virtual destructor. */
	virtual ~Work_Order() {};
	

	// Public Methods
	/** Begins reading and parsing an asset's data from disk. 
	 * @note	This is designed to be multi-threaded if the asset can support it. */
	virtual void initializeOrder() = 0;
	/** Finishes remaining operations needed to finalize an asset.
	 * @note	Since many assets require GPU synchronization, this step is done in the main thread. */
	virtual void finalizeOrder() = 0;
};

#endif // ASSET_H