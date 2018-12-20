#pragma once
#ifndef MUSIC_S_H
#define MUSIC_S_H 

#include "Modules\Game\Systems\Interface.h"
#include "Modules\Game\Components\Board_C.h"
#include "Modules\Game\Common_Definitions.h"
#include "Assets\Asset_Sound.h"
#include "Engine.h"


/** Responsible for playing music in response to game events. */
class Music_System : public Game_System_Interface {
public:
	// (de)Constructors
	~Music_System() = default;
	Music_System(Engine * engine) : m_engine(engine) {
		// Declare component types used
		addComponentType(Board_Component::ID);

		// Asset Loading
		m_soundSong = Shared_Sound(m_engine, "Game\\song.wav");
		m_soundSongCrit = Shared_Sound(m_engine, "Game\\song critical.wav");		
	}


	// Interface Implementation
	virtual bool readyToUse() override {
		return m_soundSong->existsYet() && m_soundSongCrit->existsYet();
	}
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(Board_Component*)componentParam[0];	
			const auto & soundMgr = m_engine->getSoundManager();

			if (!board.m_nearingTop) {
				if (!m_musicPlaying) {
					m_musicPlaying = true;
					m_failPlaying = false;
					soundMgr.stopWav(m_songHandle);
					m_songHandle = soundMgr.playWavBackground(m_soundSong, 0.75f, true, 3.0);
				}
			}
			else {	
				if (!m_failPlaying) {
					m_failPlaying = true;
					m_musicPlaying = false;
					soundMgr.stopWav(m_songHandle);
					m_songHandle = soundMgr.playWavBackground(m_soundSongCrit, 0.75f, true);
				}
			}
		}
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Sound m_soundSong, m_soundSongCrit;
	bool m_musicPlaying = false, m_failPlaying = false;
	unsigned int m_songHandle = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // MUSIC_S_H