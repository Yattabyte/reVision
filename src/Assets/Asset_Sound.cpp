#include "Assets\Asset_Sound.h"
#include "Engine.h"
#include "soloud_wav.h"


constexpr char* DIRECTORY_SOUNDS = "Sounds\\";

Asset_Sound::~Asset_Sound()
{
	if (m_finalized) 
		delete m_soundObj;	
}

Asset_Sound::Asset_Sound(const std::string & filename) : Asset(filename) {}

Shared_Asset_Sound Asset_Sound::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	return engine->getAssetManager().createAsset<Asset_Sound>(
		filename,
		DIRECTORY_SOUNDS,
		"",
		&initialize,
		engine,
		threaded
	);
}

void Asset_Sound::initialize(Engine * engine, const std::string & relativePath)
{
	// Forward asset creation
	SoLoud::Wav * wave = new SoLoud::Wav();
	auto & msgMgr = engine->getMessageManager();
	switch (wave->load(relativePath.c_str())) {
	case SoLoud::SO_NO_ERROR: 
		// No error
		break;
	case SoLoud::INVALID_PARAMETER:
		msgMgr.error("Asset_Sound \"" + m_filename + "\" has an invalid parameter.");
		break;
	case SoLoud::FILE_NOT_FOUND:
		msgMgr.error("Asset_Sound \"" + m_filename + "\" file does not exist.");
		break;
	case SoLoud::FILE_LOAD_FAILED:
		msgMgr.error("Asset_Sound \"" + m_filename + "\" file exists, but could not be loaded.");
		break;
	case SoLoud::UNKNOWN_ERROR:
	default:
		msgMgr.error("Asset_Sound \"" + m_filename + "\" has encountered an unknown error.");
		break;
	}; 
	m_soundObj = (SoundObj*)wave;
	Asset::finalize(engine);
}
