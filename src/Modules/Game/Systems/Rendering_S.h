#pragma once
#ifndef RENDERING_S_H
#define RENDERING_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Texture.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Engine.h"

/** Component Types Used */
#include "Modules\Game\Components\GameBoard_C.h"
#include "Modules\Game\Components\GameScore_C.h"


/***/
class Rendering_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Rendering_System() = default;
	Rendering_System(Engine * engine) {
		// Declare component types used
		addComponentType(GameBoard_Component::ID);
		addComponentType(GameScore_Component::ID);

		// For rendering tiles to the board
		m_orthoProj = glm::ortho<float>(0, 128 * 6, 0, 128 * 12, -1, 1);

		// Board FBO & Texture Creation
		constexpr unsigned int tileSize = 128u;
		glCreateFramebuffers(1, &m_fboID);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_boardTexID);
		glTextureImage2DEXT(m_boardTexID, GL_TEXTURE_2D, 0, GL_RGBA16F, tileSize * 6, tileSize * 12, 0, GL_RGBA, GL_FLOAT, NULL);
		glTextureParameteri(m_boardTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_boardTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_boardTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_boardTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameterf(m_boardTexID, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
		glNamedFramebufferTexture(m_fboID, GL_COLOR_ATTACHMENT0, m_boardTexID, 0);
		glNamedFramebufferDrawBuffer(m_fboID, GL_COLOR_ATTACHMENT0);

		// Error Reporting
		const GLenum Status = glCheckNamedFramebufferStatus(m_fboID, GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
			engine->getMessageManager().error("Game Board Framebuffer is incomplete. Reason: \n" + std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));		
		if (!glIsTexture(m_boardTexID))
			engine->getMessageManager().error("Game Board Texture is incomplete.");

		// Asset Loading
		m_shaderBoard = Asset_Shader::Create(engine, "Game\\Board");
		m_shaderTiles = Asset_Shader::Create(engine, "Game\\Tiles", true);
		m_shaderTileScored = Asset_Shader::Create(engine, "Game\\TileScored", true);
		m_shaderScore = Asset_Shader::Create(engine, "Game\\Score", true);
		m_shaderStop = Asset_Shader::Create(engine, "Game\\Stop", true);
		m_textureTile = Asset_Texture::Create(engine, "Game\\tile.png");
		m_textureTileScored = Asset_Texture::Create(engine, "Game\\tileScored.png");
		m_textureTilePlayer = Asset_Texture::Create(engine, "Game\\player.png");
		m_textureScoreNums = Asset_Texture::Create(engine, "Game\\scoreNums.png");
		m_shapeQuad = Asset_Primitive::Create(engine, "quad");

		// Preferences
		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint & quadSize = (GLuint)m_shapeQuad->getSize();
			// count, primCount, first, reserved
			const GLuint tileData[4] = { quadSize, (12 * 6) + 2, 0, 0 };
			m_bufferIndirectTiles = StaticBuffer(sizeof(GLuint) * 4, tileData, 0);
			const GLuint tileScoreData[4] = { quadSize, 0, 0, 0 };
			m_bufferIndirectTilesScored = StaticBuffer(sizeof(GLuint) * 4, tileScoreData, GL_DYNAMIC_STORAGE_BIT);
			const GLuint boardData[4] = { quadSize, 1, 0, 0 };
			m_bufferIndirectBoard = StaticBuffer(sizeof(GLuint) * 4, boardData, 0);
			const GLuint scoreData[4] = { quadSize, 1, 0, 0 };
			m_bufferIndirectScore = StaticBuffer(sizeof(GLuint) * 4, scoreData);
			const GLuint stopData[4] = { quadSize, 5, 0, 0 };
			m_bufferIndirectStop = StaticBuffer(sizeof(GLuint) * 4, stopData, 0);
		});
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		if (!m_shapeQuad->existsYet() ||
			!m_shaderTiles->existsYet() ||
			!m_shaderBoard->existsYet() ||
			!m_shaderScore->existsYet() ||
			!m_shaderStop->existsYet() ||
			!m_textureTile->existsYet() ||
			!m_textureScoreNums->existsYet())
			return;

		for each (const auto & componentParam in components) {
			auto & board = *(GameBoard_Component*)componentParam[0];
			auto & score = *(GameScore_Component*)componentParam[1];

			// Update Rendering Data
			constexpr int decimalPlaces[8] = { 10000000,1000000,100000,10000,1000,100,10,1 };
			GLuint scoreLength = 1;
			for (GLuint x = 0; x < 8; ++x)
				if (score.m_score >= decimalPlaces[x]) {
					scoreLength = 8-x;
					break;
				}
			const GLuint doubledLength = scoreLength * 2u;
			m_bufferIndirectScore.write(sizeof(GLuint), sizeof(GLuint), &doubledLength);
			m_shaderScore->setUniform(0, scoreLength);

			const GLuint scoreCount = (GLuint)score.m_scoredTiles.size() * 2u;
			m_bufferIndirectTilesScored.write(sizeof(GLuint), sizeof(GLuint), &scoreCount);
			m_shaderTileScored->setUniform(4, (GLuint)score.m_scoredTiles.size());
			for (size_t x = 0; x < score.m_scoredTiles.size(); ++x) {
				const glm::ivec3 data(score.m_scoredTiles[x].first[0].x, score.m_scoredTiles[x].first[0].y, (int)score.m_scoredTiles[x].first.size());
				m_bufferScoredTileMarkers.write(sizeof(glm::ivec4) * x, sizeof(glm::ivec3), &data);
			}
			
			// Render the tiles
			glViewport(0, 0, 128 * 6, 128 * 12);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
			glClear(GL_COLOR_BUFFER_BIT);
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			m_shaderTiles->bind();
			m_textureTile->bind(0);
			m_textureTilePlayer->bind(1);
			m_shaderTiles->setUniform(0, m_orthoProj);
			glBindVertexArray(m_shapeQuad->m_vaoID);
			m_bufferIndirectTiles.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// Render Score-Tiles
			m_shaderTileScored->bind();
			m_textureTileScored->bind(0);
			m_textureScoreNums->bind(1);
			m_shaderTileScored->setUniform(0, m_orthoProj);
			m_bufferIndirectTilesScored.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			m_bufferScoredTileMarkers.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 9);
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
			m_textureScoreNums->bind(0);
			m_bufferIndirectScore.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// Stop Timer
			m_shaderStop->bind();
			m_bufferIndirectStop.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// End
			glDisable(GL_BLEND);
		}
	}


private:
	// Private Attributes
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	GLuint m_fboID = 0, m_boardTexID = 0;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	glm::mat4 m_orthoProj;
	Shared_Asset_Primitive m_shapeQuad;

	// Tile Rendering Resources
	Shared_Asset_Shader m_shaderTiles, m_shaderTileScored;
	Shared_Asset_Texture m_textureTile, m_textureTilePlayer, m_textureTileScored;
	StaticBuffer m_bufferIndirectTiles, m_bufferIndirectTilesScored;
	DynamicBuffer m_bufferScoredTileMarkers;

	// Board Rendering Resources
	Shared_Asset_Shader m_shaderBoard;
	StaticBuffer m_bufferIndirectBoard;

	// Score Rendering Resources
	Shared_Asset_Shader m_shaderScore;
	Shared_Asset_Texture m_textureScoreNums;
	StaticBuffer m_bufferIndirectScore;

	// Stop-Timer Rendering Resources
	Shared_Asset_Shader m_shaderStop;
	StaticBuffer m_bufferIndirectStop;

};

#endif // RENDERING_S_H