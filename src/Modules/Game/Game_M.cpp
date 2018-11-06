#include "Modules\Game\Game_M.h"
#include "Assets\Asset_Image.h"
#include "Engine.h"
#include <atomic>

/* Component Types Used */
#include "Modules\Game\Components\GameBoard_C.h"
#include "Modules\Game\Components\GameScore_C.h"
#include "Modules\Game\Components\Player_C.h"

/* System Types Used */
#include "Modules\Game\Systems\PlayerMovement_S.h"
#include "Modules\Game\Systems\Board_S.h"
#include "Modules\Game\Systems\Score_S.h"


void Game_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getMessageManager().statement("Loading Module: Game...");

	// Board FBO & Texture Creation
	constexpr unsigned int tileSize = 128u;
	glCreateFramebuffers(1, &m_fboID);
	glCreateTextures(GL_TEXTURE_2D, 1, &m_boardTexID);
	glTextureImage2DEXT(m_boardTexID, GL_TEXTURE_2D, 0, GL_RGBA16F, tileSize * 6, tileSize * 12, 0, GL_RGBA, GL_FLOAT, NULL);
	glTextureParameteri(m_boardTexID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_boardTexID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_boardTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_boardTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_boardTexID, 0);
	glNamedFramebufferDrawBuffer(m_fboID, GL_COLOR_ATTACHMENT0);

	// Error Reporting
	const GLenum Status = glCheckNamedFramebufferStatus(m_fboID, GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
		m_engine->getMessageManager().error(MessageManager::FBO_INCOMPLETE, "Game Framebuffer", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
	if (!glIsTexture(m_boardTexID))
		m_engine->getMessageManager().error(MessageManager::TEXTURE_INCOMPLETE, "Game Texture");

	// Asset Loading
	m_shaderTiles = Asset_Shader::Create(m_engine, "Game\\Tiles", true);
	m_shaderBoard = Asset_Shader::Create(m_engine, "Game\\Board");
	m_textureTile = Asset_Texture::Create(m_engine, "Game\\tile.png");
	m_texturePlayer = Asset_Texture::Create(m_engine, "Game\\player.png");
	m_shaderScore = Asset_Shader::Create(engine, "Game\\Score", true);
	m_shaderStop = Asset_Shader::Create(engine, "Game\\Stop", true);
	m_texture7Seg = Asset_Texture::Create(engine, "Game\\7segnums.png");
	m_shapeQuad = Asset_Primitive::Create(engine, "quad");

	// Preferences
	auto & preferences = m_engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);

	// Asset-Finished Callbacks
	m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
		const GLuint & quadSize = (GLuint)m_shapeQuad->getSize();
		// count, primCount, first, reserved
		const GLuint tileData[4] = { quadSize, (12 * 6) + 2, 0, 0 }; 
		m_bufferIndirectTiles = StaticBuffer(sizeof(GLuint) * 4, tileData, 0);
		const GLuint boardData[4] = { quadSize, 1, 0, 0 };
		m_bufferIndirectBoard = StaticBuffer(sizeof(GLuint) * 4, boardData, 0);
		const GLuint scoreData[4] = { quadSize, 8, 0, 0 };
		m_bufferIndirectScore = StaticBuffer(sizeof(GLuint) * 4, scoreData, 0);		
		const GLuint stopData[4] = { quadSize, 5, 0, 0 };
		m_bufferIndirectStop = StaticBuffer(sizeof(GLuint) * 4, stopData, 0);
	});
	m_shaderTiles->addCallback(m_aliveIndicator, [&]() mutable {
		m_shaderTiles->setUniform(0, glm::ortho<float>(0, 128 * 6, 0, 128 * 12, -1, 1));
	});

	// Systems
	m_gameplaySystems.addSystem(new Board_System(m_engine));
	m_gameplaySystems.addSystem(new Score_System(m_engine));
	//m_gameplaySystems.addSystem(new PlayerMovement_System(engine));


	// Component Constructors
	m_engine->registerECSConstructor("GameBoard_Component", new GameBoard_Constructor(&m_engine->getGameModule().m_boardBuffer));
	m_engine->registerECSConstructor("GameScore_Component", new GameScore_Constructor());
	m_engine->registerECSConstructor("Player_Component", new Player_Constructor());
}

void Game_Module::tickGame(const float & deltaTime)
{
	m_timeAccumulator += deltaTime;
	m_boardBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
	
	constexpr float dt = 0.01f;
	while (m_timeAccumulator >= dt) {
		// Update ALL systems with our fixed tick rate
		m_engine->getECS().updateSystems(m_gameplaySystems, deltaTime);
		m_timeAccumulator -= dt;
	}
	
	if (!m_shapeQuad->existsYet() || 
		!m_shaderTiles->existsYet() || 
		!m_shaderBoard->existsYet() || 
		!m_shaderScore->existsYet() ||
		!m_shaderStop->existsYet() ||
		!m_textureTile->existsYet() ||
		!m_texture7Seg->existsYet() )
		return;

	// Render the tiles
	glViewport(0, 0, 128 * 6, 128 * 12);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_shaderTiles->bind();
	m_textureTile->bind(0);
	m_texturePlayer->bind(1);
	glBindVertexArray(m_shapeQuad->m_vaoID);
	m_bufferIndirectTiles.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	// Render the board
	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTextureUnit(0, m_boardTexID);
	m_shaderBoard->bind();
	m_bufferIndirectBoard.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	// Render the score
	m_shaderScore->bind();
	m_texture7Seg->bind(0);
	m_bufferIndirectScore.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	m_shaderScore->setUniform(0, 0);
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	// Stop Timer
	m_shaderStop->bind();
	m_bufferIndirectStop.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	// End
	glDisable(GL_BLEND);
}
