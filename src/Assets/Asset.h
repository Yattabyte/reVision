#pragma once
#ifndef	ASSET_H
#define	ASSET_H

#include <atomic>
#include <functional>
#include <map>
#include <stdio.h>
#include <string>
#include <utility>
#include <vector>
#include <glad/glad.h>
#undef APIENTRY


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
	std::string getFileName() const noexcept;
	/** Sets the file name of this asset.
	@param	filename	the file name to set this asset to. */
	void setFileName(const std::string& filename) noexcept;
	/** Attaches a callback method to be triggered when the asset finishes loading.
	@param	alive		a shared pointer indicating whether the caller is still alive or not.
	@param	callback	the method to be triggered. */
	void addCallback(const std::shared_ptr<bool>& alive, const AssetFinalizedCallback& callback) noexcept;
	/** Retrieves whether or not this asset has completed finalizing.
	@return				true if this asset has finished finalizing, false otherwise. */
	bool existsYet() const noexcept;


protected:
	// Protected Constructors
	/** Create asset that uses the specified file-path. */
	Asset(Engine& engine, const std::string& filename) noexcept;


	// Protected Interface
	/** Initializes the asset. */
	virtual void initialize() = 0;
	friend class AssetManager;


	// Protected Methods
	/** Declares this asset ready-to-use. */
	void finalize() noexcept;


	// Protected Attributes
	Engine& m_engine;
	std::atomic_bool m_finalized = false;
	mutable GLsync m_fence = nullptr;
	std::string m_filename = "";
	std::vector<std::pair<std::shared_ptr<bool>, std::function<void()>>> m_callbacks;


private:
	// Private but deleted
	/** Disallow asset assignment. */
	inline Asset(const Asset&) noexcept = delete;
	/** Disallow asset assignment. */
	inline const Asset& operator =(const Asset&)  noexcept = delete;
};

#endif // ASSET_H
