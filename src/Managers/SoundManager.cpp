#include "Managers/SoundManager.h"
#include "soloud.h"
#include "soloud_wav.h"


SoundManager::~SoundManager() noexcept
{
	auto* se = reinterpret_cast<SoLoud::Soloud*>(m_soundEngine);
	se->deinit();
	delete se;
}

SoundManager::SoundManager() noexcept
{
	SoLoud::Soloud* soLoud = new SoLoud::Soloud();
	soLoud->init();

	m_soundEngine = reinterpret_cast<SoundEngineObj*>(soLoud);
}

int SoundManager::GetVersion() noexcept
{
	return SOLOUD_VERSION;
}

void SoundManager::playSound(const Shared_Sound& sharedSound, const float& volume, const float& speed) const noexcept
{
	auto& soLoud = *reinterpret_cast<SoLoud::Soloud*>(m_soundEngine);
	auto handle = soLoud.play(*reinterpret_cast<SoLoud::Wav*>(sharedSound->m_soundObj), volume);
	soLoud.setRelativePlaySpeed(handle, speed);
}

unsigned int SoundManager::playWavBackground(const Shared_Sound& sharedSound, const float& volume, const bool& loop, const double& loopPoint) const noexcept
{
	auto& soLoud = *reinterpret_cast<SoLoud::Soloud*>(m_soundEngine);
	auto handle = soLoud.playBackground(*reinterpret_cast<SoLoud::Wav*>(sharedSound->m_soundObj), volume);

	if (loop) {
		soLoud.setLooping(handle, true);
		soLoud.setLoopPoint(handle, loopPoint);
	}
	return handle;
}

void SoundManager::stopWav(const unsigned int& handle) const noexcept
{
	reinterpret_cast<SoLoud::Soloud*>(m_soundEngine)->stop(handle);
}