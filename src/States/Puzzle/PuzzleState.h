#pragma once
#ifndef PUZZLESTATE_H
#define PUZZLESTATE_H

#include "States/EngineState.h"
#include "States/Puzzle/Common_Definitions.h"
#include "States/GameSystemInterface.h"
#include "Utilities/ECS/ecsSystem.h"
#include "Utilities/GL/VectorBuffer.h"
#include "Engine.h"

/* Component Types Used */
#include "States/Puzzle/ECS/Board_C.h"
#include "States/Puzzle/ECS/Score_C.h"
#include "States/Puzzle/ECS/Player2D_C.h"

/* Game System Types Used */
#include "States/Puzzle/ECS/IntroOutro_S.h"
#include "States/Puzzle/ECS/ColorScheme_S.h"
#include "States/Puzzle/ECS/Gravity_S.h"
#include "States/Puzzle/ECS/PlayerInput_S.h"
#include "States/Puzzle/ECS/Push_S.h"
#include "States/Puzzle/ECS/Score_S.h"
#include "States/Puzzle/ECS/Timer_S.h"
#include "States/Puzzle/ECS/Music.h"

/* Rendering System Types Used */
#include "States/Puzzle/ECS/Rendering_S.h"


class PuzzleState : public EngineState {
public:
	// Public (de)Constructors
	inline ~PuzzleState() = default;
	inline PuzzleState(Engine * engine) : EngineState(engine) {
		// Gameplay Systems
		m_gameplaySystems = {
			new IntroOutro_System(m_engine),
			new ColorScheme_System(),
			new Gravity_System(m_engine),
			new Push_System(),
			new PlayerInput_System(m_engine, &m_engine->getActionState()),
			new Score_System(m_engine),
			new Timer_System(),
			new Music_System(m_engine),
		};
		for each (auto * system in m_gameplaySystems)
			m_systemList.addSystem(system);

		// Rendering Systems
		m_renderingSystem = new Rendering_System(m_engine, m_engine->getModule_Graphics().getLightingFBOID());

		// Create Players
		auto & gameBoard = *m_boardBuffer.newElement();
		(*gameBoard.data) = GameBuffer();
		// Reset game buffer data
		for (int x = 0; x < BOARD_WIDTH; ++x) {
			for (int y = 0; y < BOARD_HEIGHT; ++y) {
				gameBoard.data->tiles[(y * 6) + x].type = TileState::NONE;
				gameBoard.data->tiles[(y * 6) + x].yOffset = 0.0f;
				gameBoard.data->tiles[(y * 6) + x].lifeLinear = 0.0f;
			}
			gameBoard.data->lanes[x] = 0.0f;
		}
		m_players.push_back(&gameBoard);

		// Component Constructors
		m_engine->registerECSConstructor("Board_Component", new Board_Constructor(m_players[0]));
		m_engine->registerECSConstructor("Score_Component", new Score_Constructor(m_players[0]));
		m_engine->registerECSConstructor("Player2D_Component", new Player2D_Constructor());
		m_engine->getModule_World().loadWorld("game.map");
		glfwSetInputMode(m_engine->getRenderingContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}


	// Public Interface Implementation
	inline virtual void handleTick(const float & deltaTime) override {
		// Check if safe to start game
		if (!m_readyToStart) {
			bool allReady = true;
			for each (auto * system in m_gameplaySystems)
				if (!system->readyToUse()) {
					allReady = false;
					break;
				}
			if (allReady)
				m_readyToStart = true;
		}

		// Update Game
		if (m_readyToStart) {
			m_timeAccumulator += deltaTime;
			auto & ecs = m_engine->getECS();
			constexpr float dt = 1.0f / 60.0f;
			while (m_timeAccumulator >= dt) {
				// Update ALL systems using a fixed tick rate
				ecs.updateSystems(m_systemList, dt);
				m_timeAccumulator -= dt;
			}

			// Render Game
			m_engine->getModule_Graphics().setActiveCamera(0);
			m_engine->getModule_Graphics().frameTick(deltaTime);
			m_boardBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
			ecs.updateSystem(m_renderingSystem, deltaTime);
			m_engine->getModule_PostProcess().frameTick(deltaTime);
		}
	}


protected:
	// Protected Attributes
	bool m_readyToStart = false;
	float m_timeAccumulator = 0.0f;
	std::vector<Game_System_Interface*> m_gameplaySystems;
	ECSSystemList m_systemList;
	BaseECSSystem * m_renderingSystem;
	VectorBuffer<GameBuffer> m_boardBuffer;
	std::vector<VB_Element<GameBuffer>*> m_players;
};

#endif // PUZZLESTATE_H