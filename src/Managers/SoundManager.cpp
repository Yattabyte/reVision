#include "Managers/SoundManager.h"
#include "soloud.h"
#include "soloud_wav.h"


SoundManager::~SoundManager()
{
	((SoLoud::Soloud*)soundEngine)->deinit();
	delete soundEngine;
}

SoundManager::SoundManager()
{
	SoLoud::Soloud * soLoud = new SoLoud::Soloud();
	soLoud->init();

	soundEngine = (SoundEngineObj*)soLoud;
}

void SoundManager::playWav(const SoundObj * const obj, const float & volume, const float & speed, const double & time) const
{
	auto & soLoud = *((SoLoud::Soloud*)soundEngine);	
	auto handle = soLoud.play(*(SoLoud::Wav*)obj, volume);
	soLoud.setRelativePlaySpeed(handle, speed);
}

const unsigned int SoundManager::playWavBackground(const SoundObj * const obj, const float & volume, const bool & loop, const double & loopPoint) const
{
	auto & soLoud = *((SoLoud::Soloud*)soundEngine);
	auto handle = soLoud.playBackground(*(SoLoud::Wav*)obj, volume);

	if (loop) {
		soLoud.setLooping(handle, true);
		soLoud.setLoopPoint(handle, loopPoint);
	}
	return handle;
}

void SoundManager::stopWavBackground(const unsigned int & handle) const
{
	(*((SoLoud::Soloud*)soundEngine)).stop(handle);
}