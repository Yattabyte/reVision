#pragma once
#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include "Assets/Sound.h"


class SoundEngineObj;
class SoundObj;

/** API for playing sound assets in the engine. */
class SoundManager {
public:
	// Public (De)Constructors
	/** Destroy the sound manager. */
	~SoundManager() noexcept;
	/** Construct the sound manager. */
	SoundManager() noexcept;


	// Public Methods
	/** Retrieve the current version of the sound engine used.
	@return					numeric version number. */
	static int GetVersion() noexcept;
	/** Play a sound wave file.
	@param		sharedSound	the sound file.
	@param		volume		the volume. (default == -1 means == use default).
	@param		speed		the playback speed. (default == 1.0f == normal speed).
	@return					handle to the playing track. */
	void playSound(const Shared_Sound& sharedSound, const float& volume = -1.0f, const float& speed = 1.0f) const noexcept;
	/** Play a sound wave file as background audio.
	@param		sharedSound	the sound file.
	@param		volume		the volume. (default == -1 means == use default).
	@param		loop		whether to loop the track or not.
	@param		loopPoint	if looping, the point in seconds to loop back to (default == 0.0 == beginning).
	@return					handle to the playing track. */
	unsigned int playWavBackground(const Shared_Sound& sharedSound, const float& volume = 1.0f, const bool& loop = false, const double& loopPoint = 0.0) const noexcept;
	/** Stops a playing track from playing.
	@param		handle		the handle to the playing track. */
	void stopWav(const unsigned int& handle) const noexcept;


private:
	SoundEngineObj* m_soundEngine = nullptr;
};

#endif // SOUNDMANAGER_H
