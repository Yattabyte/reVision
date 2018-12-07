#pragma once
#ifndef	ASSET_SOUND_H
#define	ASSET_SOUND_H

#include "Assets\Asset.h"


class Engine;
class Asset_Sound;
class SoundObj;
using Shared_Asset_Sound = std::shared_ptr<Asset_Sound>;

/** A Sound object. */
class Asset_Sound : public Asset
{
public:
	/** Destroy the Sound. */
	~Asset_Sound();
	/** Construct the Sound. */
	Asset_Sound(const std::string & filename);


	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	static Shared_Asset_Sound Create(Engine * engine, const std::string & filename, const bool & threaded = true);


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