#include "Assets/Sound.h"
#include "Engine.h"
#include "soloud_wav.h"


constexpr char* DIRECTORY_SOUNDS = "Sounds\\";

Shared_Sound::Shared_Sound(Engine * engine, const std::string & filename, const bool & threaded)
	: std::shared_ptr<Sound>(std::dynamic_pointer_cast<Sound>(engine->getManager_Assets().shareAsset(typeid(Sound).name(), filename)))
{
	// Find out if the asset needs to be created
	if (!get()) {
		// Create new asset on shared_ptr portion of this class 
		(*(std::shared_ptr<Sound>*)(this)) = std::make_shared<Sound>(filename);
		// Submit data to asset manager
		engine->getManager_Assets().submitNewAsset(typeid(Sound).name(), (*(std::shared_ptr<Asset>*)(this)), std::move(std::bind(&Sound::initialize, get(), engine, (DIRECTORY_SOUNDS + filename))), threaded);
	}
	// Check if we need to wait for initialization
	else
		if (!threaded)
			// Stay here until asset finalizes
			while (!get()->existsYet())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
}


Sound::~Sound()
{
	if (m_finalized) 
		delete (SoLoud::Wav*)m_soundObj;
}

Sound::Sound(const std::string & filename) : Asset(filename) {}

void Sound::initialize(Engine * engine, const std::string & relativePath)
{
	// Forward asset creation
	SoLoud::Wav * wave = new SoLoud::Wav();
	auto & msgMgr = engine->getManager_Messages();
	switch (wave->load(relativePath.c_str())) {
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
	Asset::finalize(engine);
}