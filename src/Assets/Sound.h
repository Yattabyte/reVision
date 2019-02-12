#pragma once
#ifndef	SOUND_H
#define	SOUND_H

#include "Assets/Asset.h"


class Engine;
class Sound;
class SoundObj;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Sound : public std::shared_ptr<Sound> {
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
class Sound : public Asset
{
public:
	/** Destroy the Sound. */
	~Sound();
	/** Construct the Sound. */
	Sound(Engine * engine, const std::string & filename);


	// Public Attributes
	SoundObj * m_soundObj;


protected:
	// Private Methods
	// Interface Implementation
	virtual void initialize() override;


	// Private Attributes
	friend class Shared_Sound;
};

#endif // SOUND_H