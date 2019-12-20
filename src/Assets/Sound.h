#pragma once
#ifndef	SOUND_H
#define	SOUND_H

#include "Assets/Asset.h"


// Forward Declarations
class Sound;
class SoundObj;

/** Shared version of a Sound asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Sound final : public std::shared_ptr<Sound> {
public:
	// Public (De)Constructors
	/** Constructs an empty asset. */
	inline Shared_Sound() noexcept = default;
	/** Begins the creation process for this asset.
	@param	engine			reference to the engine to use. 
	@param	filename		the filename to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	explicit Shared_Sound(Engine& engine, const std::string& filename, const bool& threaded = true);
};

/** A sound byte object.
Represents a sound file loaded from disk.
@note	requires the SoLoud library to use. */
class Sound : public Asset {
public:
	// Public (De)Constructors
	/** Destroy the Sound. */
	~Sound() noexcept;
	/** Construct the Sound.
	@param	engine			reference to the engine to use. 
	@param	filename		the asset file name (relative to engine directory). */
	Sound(Engine& engine, const std::string& filename);


	// Public Attributes
	SoundObj* m_soundObj = nullptr;


protected:
	// Private but deleted
	/** Disallow asset move constructor. */
	inline Sound(Sound&&) noexcept = delete;
	/** Disallow asset copy constructor. */
	inline Sound(const Sound&) noexcept = delete;
	/** Disallow asset move assignment. */
	inline const Sound& operator =(Sound&&) noexcept = delete;
	/** Disallow asset copy assignment. */
	inline const Sound& operator =(const Sound&) noexcept = delete;


	// Private Interface Implementation
	void initialize() final;


	// Private Attributes
	friend class Shared_Sound;
};

#endif // SOUND_H