#pragma once
#ifndef	ASSET_H
#define	ASSET_H

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include <glad/glad.h>
#undef APIENTRY


// Forward Declarations
class Asset;
class Engine;
using Shared_Asset = std::shared_ptr<Asset>;
using AssetFinalizedCallback = std::function<void(void)>;

/** An abstract base-class for assets.
Represents some form of data to be loaded from disk, such as shaders, models, levels, and sounds.
@note	is an abstract class instead of interface to reduce redundant code.
@note	should be created once, and its pointer passed around using shared pointers. */
class Asset {
public:
	// Public (De)Constructors
	/** Destroy the asset only when all references are destroyed. */
	inline virtual ~Asset() noexcept = default;


	// Public Methods
	/** Retrieves the file name of this asset.
	@return				the file name belonging to this asset. */
	std::string getFileName() const;
	/** Sets the file name of this asset.
	@param	filename	the file name to set this asset to. */
	void setFileName(const std::string& filename);
	/** Attaches a callback method to be triggered when the asset finishes loading.
	@param	alive		a shared pointer indicating whether the caller is still alive or not.
	@param	callback	the method to be triggered. */
	void addCallback(const std::shared_ptr<bool>& alive, const AssetFinalizedCallback& callback);
	/** Retrieves whether or not this asset has completed finalizing.
	@return				true if this asset has finished finalizing, false otherwise. */
	bool ready() const noexcept;
	/** Check if an input variadic list of shared assets have all completed finalizing. 
	@param	<>			variadic list of assets to check (auto-deducible).
	@param	firstAsset	the first value to check.
	@param	...rest		the rest of the values to check.
	@return				true if all the assets have finished finalizing, false otherwise. */
	template <typename FirstAsset, typename ...RemainingAssets>
	inline static bool All_Ready(const FirstAsset& firstAsset, const RemainingAssets& ...rest) noexcept {
		// Ensure all inputs are shared assets
		static_assert(!std::is_base_of<std::shared_ptr<Asset>, FirstAsset>::value, "Asset::All_Ready(...) parameter is not a Shared_Asset!");

		// Proceed only if the first asset is ready, recursively
		if (firstAsset->ready()) {
			// For each remaining member of the parameter pack, recursively call this function
			if constexpr (sizeof...(rest) > 0)
				return All_Ready(rest...);
			else
				return true;
		}

		return false;
	}


protected:
	// Protected Constructors
	/** Create asset that uses the specified file-path. */
	Asset(Engine& engine, const std::string& filename);


	// Protected Interface
	/** Initializes the asset. */
	virtual void initialize() = 0;
	friend class AssetManager;


	// Protected Methods
	/** Declares this asset ready-to-use. */
	void finalize();


	// Protected Attributes
	Engine& m_engine;
	std::atomic_bool m_finalized = false;
	mutable GLsync m_fence = nullptr;
	std::string m_filename = "";
	std::vector<std::pair<std::shared_ptr<bool>, std::function<void()>>> m_callbacks;


private:
	// Private but deleted
	/** Disallow asset move constructor. */
	inline Asset(Asset&&) noexcept = delete;
	/** Disallow asset copy constructor. */
	inline Asset(const Asset&) noexcept = delete;
	/** Disallow asset move assignment. */
	inline const Asset& operator =(Asset&&) noexcept = delete;
	/** Disallow asset copy assignment. */
	inline const Asset& operator =(const Asset&) noexcept = delete;
};

#endif // ASSET_H