#include "Assets/Sound.h"
#include "Engine.h"
#include "soloud_wav.h"


constexpr char* DIRECTORY_SOUNDS = "Sounds\\";

Shared_Sound::Shared_Sound(Engine* engine, const std::string& filename, const bool& threaded)
{
	(*(std::shared_ptr<Sound>*)(this)) = std::dynamic_pointer_cast<Sound>(
		engine->getManager_Assets().shareAsset(
			typeid(Sound).name(),
			filename,
			[engine, filename]() { return std::make_shared<Sound>(engine, filename); },
			threaded
		));
}

Sound::~Sound()
{
	if (m_finalized)
		delete (SoLoud::Wav*)m_soundObj;
}

Sound::Sound(Engine* engine, const std::string& filename) : Asset(engine, filename) {}

void Sound::initialize()
{
	// Forward asset creation
	SoLoud::Wav* wave = new SoLoud::Wav();
	auto& msgMgr = m_engine->getManager_Messages();
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
	default:
		msgMgr.error("Sound \"" + m_filename + "\" has encountered an unknown error.");
		break;
	};
	m_soundObj = (SoundObj*)wave;
	Asset::finalize();
}