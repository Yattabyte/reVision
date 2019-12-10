#include "Assets/Sound.h"
#include "soloud_wav.h"
#include "Engine.h"


constexpr const char* DIRECTORY_SOUNDS = "Sounds\\";

Shared_Sound::Shared_Sound(Engine& engine, const std::string& filename, const bool& threaded) noexcept
{
	swap(std::dynamic_pointer_cast<Sound>(engine.getManager_Assets().shareAsset(
			typeid(Sound).name(),
			filename,
			[&engine, filename]() { return std::make_shared<Sound>(engine, filename); },
			threaded
		)));
}

Sound::~Sound() noexcept
{
	if (m_finalized)
		delete reinterpret_cast<SoLoud::Wav*>(m_soundObj);
}

Sound::Sound(Engine& engine, const std::string& filename) noexcept : Asset(engine, filename) {}

void Sound::initialize() noexcept
{
	// Forward asset creation
	auto* wave = new SoLoud::Wav();
	auto& msgMgr = m_engine.getManager_Messages();
	const auto path = DIRECTORY_SOUNDS + getFileName();
	switch (wave->load(path.c_str())) {
	case SoLoud::SO_NO_ERROR:
		// No error
		break;
	case SoLoud::INVALID_PARAMETER:
		msgMgr.error("Sound \"" + m_filename + "\" has an invalid parameter.");
		break;
	case SoLoud::FILE_NOT_FOUND:
		msgMgr.error("Sound \"" + m_filename + "\" file does not exist.");
		break;
	case SoLoud::FILE_LOAD_FAILED:
		msgMgr.error("Sound \"" + m_filename + "\" file exists, but could not be loaded.");
		break;
	case SoLoud::UNKNOWN_ERROR:
		[[fallthrough]];
	default:
		msgMgr.error("Sound \"" + m_filename + "\" has encountered an unknown error.");
		break;
	};
	m_soundObj = reinterpret_cast<SoundObj*>(wave);
	Asset::finalize();
}