#pragma once
#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

class SoundEngineObj;
class SoundObj;

/** API for playing sound assets in the engine. */
class SoundManager {
public:
	// (de)Constructors
	/** Destroy the sound manager. */
	~SoundManager();
	/** Construct the sound manager*/
	SoundManager();


	// Public Methods
	/** Play a sound wave file. 
	@param		obj		the sound file. */
	void playWav(const SoundObj * const obj) const;


private:
	SoundEngineObj * soundEngine;
};

#endif // SOUNDMANAGER_H