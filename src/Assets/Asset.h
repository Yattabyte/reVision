#pragma once
#ifndef	ASSET_H
#define	ASSET_H

#include "GL\glew.h"
#include <atomic>
#include <functional>
#include <map>
#include <stdio.h>
#include <string>
#include <utility>
#include <vector>


class Asset;
class Engine;
using Shared_Asset = std::shared_ptr<Asset>;
using AssetFinalizedCallback = std::function<void(void)>;

/** An abstract base-class for assets.
@brief	Represents some form of data to be loaded from disk, such as shaders, models, levels, and sounds.
@note		is an abstract class instead of interface to reduce redundant code.
@usage	Should be created once, and its pointer passed around using shared pointers. */
class Asset
{
public:
	// (de)Constructors
	/** Destroy the asset only when all references are destroyed. */
	~Asset() = default;


	// Public Methods	
	/** Gets the file name of this asset.
	@return				the file name belonging to this asset */
	std::string getFileName() const;
	/** Sets the file name of this asset.
	@param	filename	the file name to set this asset to */
	void setFileName(const std::string & filename);	
	/** Attaches a callback method to be triggered when the asset finishes loading.
	@param	alive		a shared pointer indicating whether the caller is alive or not
	@param	callback	the method to be triggered */
	void addCallback(const std::shared_ptr<bool> & alive, AssetFinalizedCallback && callback);
	/** Returns whether or not this asset has completed finalizing.
	@return				true if this asset has finished finalizing, false otherwise. */
	bool existsYet() const;


protected:
	// Protected Constructors
	/** Create asset that uses the specified file-path. */
	Asset(const std::string & filename);


	// Protected Interface
	/** Initializes the asset. */
	virtual void initialize(Engine * engine, const std::string & relativePath) = 0;

	
	// Protected Methods
	/** Declares this asset ready-to-use. */
	void finalize(Engine * engine);


	// Protected Attributes
	std::atomic_bool m_finalized = false;
	mutable GLsync m_fence = nullptr;
	std::string m_filename = "";
	std::vector<std::pair<std::shared_ptr<bool>, std::function<void()>>> m_callbacks;
	friend class AssetManager;

	
private:
	// Private but deleted
	Asset(const Asset &) = delete;
	const Asset &operator =(const Asset &) = delete;
};

#endif // ASSET_H