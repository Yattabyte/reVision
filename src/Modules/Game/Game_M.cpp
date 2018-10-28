#include "Modules\Game\Game_M.h"
#include "Assets\Asset_Image.h"
#include "Engine.h"
#include <atomic>

/* Component Types Used */
#include "Modules\Game\Components\Player_C.h"
#include "Modules\Game\Components\BoardState_C.h"

/* System Types Used */
#include "Modules\Game\Systems\PlayerMovement_S.h"
#include "Modules\Game\Systems\Board_S.h"


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
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_blockTextureID);
	glTextureImage3DEXT(m_blockTextureID, GL_TEXTURE_2D_ARRAY, 0, GL_RGBA16F, 128, 128, 6, 0, GL_RGBA, GL_FLOAT, NULL);
	glTextureParameteri(m_blockTextureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_blockTextureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_blockTextureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_blockTextureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Error Reporting
	const GLenum Status = glCheckNamedFramebufferStatus(m_fboID, GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
		m_engine->getMessageManager().error(MessageManager::FBO_INCOMPLETE, "Game Framebuffer", std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
	if (!glIsTexture(m_boardTexID))
		m_engine->getMessageManager().error(MessageManager::TEXTURE_INCOMPLETE, "Game Texture");
	if (!glIsTexture(m_blockTextureID))
		m_engine->getMessageManager().error(MessageManager::TEXTURE_INCOMPLETE, "Game Block Texture");

	// Asset Loading
	m_shaderTiles = Asset_Shader::Create(m_engine, "Game\\Tiles");
	m_shaderBoard = Asset_Shader::Create(m_engine, "Game\\Board");
	m_shapeQuad = Asset_Primitive::Create(engine, "quad");
	// Combine Block Textures
	constexpr const char* imageNames[6] = { "\\Textures\\Game\\A.png", "\\Textures\\Game\\B.png", "\\Textures\\Game\\C.png", "\\Textures\\Game\\D.png", "\\Textures\\Game\\E.png", "\\Textures\\Game\\P.png" };
	for (int x = 0; x < 6; ++x) {
		auto image = Asset_Image::Create(m_engine, imageNames[x], glm::ivec2(128u));
		image->addCallback(m_aliveIndicator, [&textureID = m_blockTextureID, image, x]() {
			glTextureSubImage3D(textureID, 0, 0, 0, x, 128, 128, 1, GL_RGBA, GL_UNSIGNED_BYTE, image->m_pixelData);
		});
	}

	// Preferences
	auto & preferences = m_engine->getPreferenceState();
	preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
	preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);

	// Asset-Finished Callbacks
	m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
		const GLuint tileData[4] = { (GLuint)m_shapeQuad->getSize(), (12 * 6) + 2, 0, 0 }; // count, primCount, first, reserved
		const GLuint boardData[4] = { (GLuint)m_shapeQuad->getSize(), 1, 0, 0 }; // count, primCount, first, reserved
		m_quad_tiles_indirect = StaticBuffer(sizeof(GLuint) * 4, tileData, 0);
		m_quad_board_indirect = StaticBuffer(sizeof(GLuint) * 4, boardData, 0);
		
	});
	m_shaderTiles->addCallback(m_aliveIndicator, [&]() mutable {
		m_shaderTiles->setUniform(0, glm::ortho<float>(0, 128 * 6, 0, 128 * 12, -1, 1));
	});

	// Systems
	m_gameplaySystems.addSystem(new Board_System(m_engine));
	//m_gameplaySystems.addSystem(new PlayerMovement_System(engine));


	// Component Constructors
	m_engine->registerECSConstructor("Player_Component", new Player_Constructor());
	m_engine->registerECSConstructor("BoardState_Component", new BoardState_Constructor(&m_engine->getGameModule().m_boardBuffer));
}

void Game_Module::tickGame(const float & deltaTime)
{
	m_engine->getECS().updateSystems(m_gameplaySystems, deltaTime);
	
	if (!m_shapeQuad->existsYet() || !m_shaderTiles->existsYet() || !m_shaderBoard->existsYet())
		return;

	glViewport(0, 0, 128 * 6, 128 * 12);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	m_boardBuffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 8);
	m_shaderTiles->bind();
	glBindTextureUnit(0, m_blockTextureID);
	glBindVertexArray(m_shapeQuad->m_vaoID);
	m_quad_tiles_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, 0);

	glViewport(0, 0, m_renderSize.x, m_renderSize.y);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTextureUnit(0, m_boardTexID);
	m_shaderBoard->bind();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	m_quad_board_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
	glDrawArraysIndirect(GL_TRIANGLES, 0);
	glDisable(GL_BLEND);
}
