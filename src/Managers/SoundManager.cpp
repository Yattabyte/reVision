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

void SoundManager::playWav(const SoundObj * const obj) const
{
	auto & soLoud = *((SoLoud::Soloud*)soundEngine);
	soLoud.play(*(SoLoud::Wav*)obj);
}
