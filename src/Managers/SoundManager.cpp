#include "Managers/SoundManager.h"
#include "soloud.h"
#include "soloud_wav.h"


SoundManager::~SoundManager()
{
	auto* se = ((SoLoud::Soloud*)soundEngine);
	se->deinit();
	delete se;
}

SoundManager::SoundManager()
{
	SoLoud::Soloud* soLoud = new SoLoud::Soloud();
	soLoud->init();

	soundEngine = (SoundEngineObj*)soLoud;
}

int SoundManager::GetVersion()
{
	return SOLOUD_VERSION;
}

void SoundManager::playSound(const Shared_Sound& sharedSound, const float& volume, const float& speed) const
{
	auto& soLoud = *((SoLoud::Soloud*)soundEngine);
	auto handle = soLoud.play(*(SoLoud::Wav*)sharedSound->m_soundObj, volume);
	soLoud.setRelativePlaySpeed(handle, speed);
}

unsigned int SoundManager::playWavBackground(const Shared_Sound& sharedSound, const float& volume, const bool& loop, const double& loopPoint) const
{
	auto& soLoud = *((SoLoud::Soloud*)soundEngine);
	auto handle = soLoud.playBackground(*(SoLoud::Wav*)sharedSound->m_soundObj, volume);

	if (loop) {
		soLoud.setLooping(handle, true);
		soLoud.setLoopPoint(handle, loopPoint);
	}
	return handle;
}

void SoundManager::stopWav(const unsigned int& handle) const
{
	(*((SoLoud::Soloud*)soundEngine)).stop(handle);
}