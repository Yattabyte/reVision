#pragma once
#ifndef MUSIC_S_H
#define MUSIC_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Assets\Asset_Sound.h"
#include "Modules\Game\Common.h"
#include "Engine.h"

/** Component Types Used */
#include "Modules\Game\Components\GameBoard_C.h"


/** Responsible for playing music in response to game events. */
class Music_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Music_System() = default;
	Music_System(Engine * engine) : m_engine(engine) {
		// Declare component types used
		addComponentType(GameBoard_Component::ID);

		// Asset Loading
		m_soundSongGood = Asset_Sound::Create(m_engine, "Game\\song.wav");
		m_soundSongBad = Asset_Sound::Create(m_engine, "Game\\song fail.wav");
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto & board = *(GameBoard_Component*)componentParam[0];	
			const auto & soundMgr = m_engine->getSoundManager();

			if (!board.m_nearingTop) {
				if (!m_musicPlaying && m_soundSongGood->existsYet()) {
					m_musicPlaying = true;
					m_failPlaying = false;
					soundMgr.stopWavBackground(m_songHandle);
					m_songHandle = soundMgr.playWavBackground(m_soundSongGood->m_soundObj, 0.75f, true, 3.0);
				}
			}
			else {	
				if (!m_failPlaying && m_soundSongBad->existsYet()) {
					m_failPlaying = true;
					m_musicPlaying = false;
					soundMgr.stopWavBackground(m_songHandle);
					m_songHandle = soundMgr.playWavBackground(m_soundSongBad->m_soundObj, 0.75f, true);
				}
			}
		}
	}


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	Shared_Asset_Sound m_soundSongGood, m_soundSongBad;
	bool m_musicPlaying = false, m_failPlaying = false;
	unsigned int m_songHandle = 0;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // MUSIC_S_H