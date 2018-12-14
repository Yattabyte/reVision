#pragma once
#ifndef	ASSET_SOUND_H
#define	ASSET_SOUND_H

#include "Assets\Asset.h"


class Engine;
class Asset_Sound;
class SoundObj;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Sound : public std::shared_ptr<Asset_Sound> {
public:
	Shared_Sound() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Sound(Engine * engine, const std::string & filename, const bool & threaded = true);
};

/** A Sound object. */
class Asset_Sound : public Asset
{
public:
	/** Destroy the Sound. */
	~Asset_Sound();
	/** Construct the Sound. */
	Asset_Sound(const std::string & filename);


	// Public Attributes
	SoundObj * m_soundObj;


protected:
	// Private Methods
	// Interface Implementation
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_SOUND_H