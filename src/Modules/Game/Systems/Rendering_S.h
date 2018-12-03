#pragma once
#ifndef RENDERING_S_H
#define RENDERING_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Model.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Texture.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Engine.h"

/** Component Types Used */
#include "Modules\Game\Components\GameBoard_C.h"
#include "Modules\Game\Components\GameScore_C.h"


constexpr unsigned int tileSize = 128u;

/***/
class Rendering_System : public BaseECSSystem {
public:
	// (de)Constructors
	~Rendering_System() = default;
	Rendering_System(Engine * engine, const GLuint & lightingFBOID) : m_lightingFBOID(lightingFBOID) {
		// Declare component types used
		addComponentType(GameBoard_Component::ID);
		addComponentType(GameScore_Component::ID);
		
		m_vaoModels = &engine->getModelManager().getVAO();

		// For rendering tiles to the board
		m_orthoProjField = glm::ortho<float>(0, tileSize * 6, 0, tileSize * 12, -1, 1);
		m_orthoProjHeader = glm::ortho<float>(-384, 384, -96, 96, -1, 1);

		// FBO & Texture Creation
		glCreateFramebuffers(1, &m_fboIDBorder);
		glCreateTextures(GL_TEXTURE_1D, 1, &m_borderTexID);
		glTextureImage1DEXT(m_borderTexID, GL_TEXTURE_1D, 0, GL_RGBA16F, tileSize * 16, 0, GL_RGBA, GL_FLOAT, NULL);
		glTextureParameteri(m_borderTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_borderTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_borderTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameterf(m_borderTexID, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
		glNamedFramebufferTexture(m_fboIDBorder, GL_COLOR_ATTACHMENT0, m_borderTexID, 0);
		glNamedFramebufferDrawBuffer(m_fboIDBorder, GL_COLOR_ATTACHMENT0);

		glCreateFramebuffers(1, &m_fboIDField);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_boardTexID);
		glTextureImage2DEXT(m_boardTexID, GL_TEXTURE_2D, 0, GL_RGBA16F, tileSize * 6, tileSize * 12, 0, GL_RGBA, GL_FLOAT, NULL);
		glTextureParameteri(m_boardTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_boardTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_boardTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_boardTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameterf(m_boardTexID, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
		glNamedFramebufferTexture(m_fboIDField, GL_COLOR_ATTACHMENT0, m_boardTexID, 0);
		glNamedFramebufferDrawBuffer(m_fboIDField, GL_COLOR_ATTACHMENT0);

		glCreateFramebuffers(1, &m_fboIDBars);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_scoreTexID);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_timeTexID);
		glTextureImage2DEXT(m_scoreTexID, GL_TEXTURE_2D, 0, GL_RGBA16F, tileSize * 6, tileSize * 1.5, 0, GL_RGBA, GL_FLOAT, NULL);
		glTextureImage2DEXT(m_timeTexID, GL_TEXTURE_2D, 0, GL_RGBA16F, tileSize * 6, tileSize * 1.5, 0, GL_RGBA, GL_FLOAT, NULL);
		glTextureParameteri(m_scoreTexID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_scoreTexID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_scoreTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_scoreTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_timeTexID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_timeTexID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_timeTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_timeTexID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(m_fboIDBars, GL_COLOR_ATTACHMENT0, m_scoreTexID, 0);
		glNamedFramebufferTexture(m_fboIDBars, GL_COLOR_ATTACHMENT1, m_timeTexID, 0);
		const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glNamedFramebufferDrawBuffers(m_fboIDBars, 2, drawBuffers);

		// Error Reporting
		GLenum Status = glCheckNamedFramebufferStatus(m_fboIDField, GL_FRAMEBUFFER);
		if (!glIsTexture(m_borderTexID))
			engine->getMessageManager().error("Game Border Texture is incomplete.");
		//if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
		//	engine->getMessageManager().error("Game Board Framebuffer is incomplete. Reason: \n" + std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));		
		if (!glIsTexture(m_boardTexID))
			engine->getMessageManager().error("Game Board Texture is incomplete.");
		Status = glCheckNamedFramebufferStatus(m_fboIDBars, GL_FRAMEBUFFER);
		//if (Status != GL_FRAMEBUFFER_COMPLETE && Status != GL_NO_ERROR)
		//	engine->getMessageManager().error("Game Header Framebuffer is incomplete. Reason: \n" + std::string(reinterpret_cast<char const *>(glewGetErrorString(Status))));
		if (!glIsTexture(m_scoreTexID))
			engine->getMessageManager().error("Game Header Texture is incomplete.");

		// Asset Loading
		m_modelBoard = Asset_Model::Create(engine, "Game\\boardBorder.obj");
		m_modelField = Asset_Model::Create(engine, "Game\\boardField.obj");
		m_modelHeader = Asset_Model::Create(engine, "Game\\boardTop.obj");
		m_modelFooter = Asset_Model::Create(engine, "Game\\boardBottom.obj");
		m_shaderBorder = Asset_Shader::Create(engine, "Game\\Border", true);
		m_shaderBoard = Asset_Shader::Create(engine, "Game\\Board", true);
		m_shaderTiles = Asset_Shader::Create(engine, "Game\\Tiles", true);
		m_shaderMatchedTiles = Asset_Shader::Create(engine, "Game\\Matched", true);
		m_shaderMatchedNo = Asset_Shader::Create(engine, "Game\\MatchedNumber", true);
		m_shaderScore = Asset_Shader::Create(engine, "Game\\Score", true);
		m_shaderMultiplier = Asset_Shader::Create(engine, "Game\\Multiplier", true);
		m_shaderTimer = Asset_Shader::Create(engine, "Game\\Timer", true);
		m_textureTile = Asset_Texture::Create(engine, "Game\\tile.png");
		m_textureTilePlayer = Asset_Texture::Create(engine, "Game\\player.png");
		m_textureMatchedTiles = Asset_Texture::Create(engine, "Game\\scorePieces.png");
		m_textureNums = Asset_Texture::Create(engine, "Game\\numbers.png");
		m_textureTimeStop = Asset_Texture::Create(engine, "Game\\timestop.png");
		m_shapeQuad = Asset_Primitive::Create(engine, "quad");

		// Preferences
		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint & quadSize = (GLuint)m_shapeQuad->getSize();
			// count, primCount, first, reserved
			const GLuint borderData[4] = { quadSize, 1, 0, 0 };
			m_bufferIndirectBorder = StaticBuffer(sizeof(GLuint) * 4, borderData, 0);
			const GLuint tileData[4] = { quadSize, (12 * 6) + 2, 0, 0 };
			m_bufferIndirectTiles = StaticBuffer(sizeof(GLuint) * 4, tileData, 0);
			const GLuint tileScoreData[4] = { quadSize, 0, 0, 0 };
			m_bufferIndirectMatchedNo = StaticBuffer(sizeof(GLuint) * 4, tileScoreData, GL_DYNAMIC_STORAGE_BIT);
			const GLuint tileMatchedData[4] = { quadSize, 0, 0, 0 };
			m_bufferIndirectMatchedTiles = StaticBuffer(sizeof(GLuint) * 4, tileMatchedData, GL_DYNAMIC_STORAGE_BIT);
			const GLuint scoreData[4] = { quadSize, 1, 0, 0 };
			m_bufferIndirectScore = StaticBuffer(sizeof(GLuint) * 4, scoreData);
			const GLuint stopData[4] = { quadSize, 6, 0, 0 };
			m_bufferIndirectStop = StaticBuffer(sizeof(GLuint) * 4, stopData, 0);
			const GLuint multiplierData[4] = { quadSize, 4, 0, 0 };
			m_bufferIndirectMultiplier = StaticBuffer(sizeof(GLuint) * 4, multiplierData, 0);
		});
		m_bufferIndirectBoard = StaticBuffer(sizeof(GLint) * 16, 0, GL_DYNAMIC_STORAGE_BIT);
		m_modelBoard->addCallback(m_aliveIndicator, [&]() mutable {
			const GLint data[4] = { m_modelBoard->m_count, 1, m_modelBoard->m_offset, 1 };
			m_bufferIndirectBoard.write(0, sizeof(GLint) * 4, data);
		});
		m_modelField->addCallback(m_aliveIndicator, [&]() mutable {
			const GLint data[4] = { m_modelField->m_count, 1, m_modelField->m_offset, 1 };
			m_bufferIndirectBoard.write(sizeof(GLint) * 4, sizeof(GLint) * 4, data);
		});
		m_modelHeader->addCallback(m_aliveIndicator, [&]() mutable {
			const GLint data[4] = { m_modelHeader->m_count, 1, m_modelHeader->m_offset, 1 };
			m_bufferIndirectBoard.write(sizeof(GLint) * 8, sizeof(GLint) * 4, data);
		});
		m_modelFooter->addCallback(m_aliveIndicator, [&]() mutable {
			const GLint data[4] = { m_modelFooter->m_count, 1, m_modelFooter->m_offset, 1 };
			m_bufferIndirectBoard.write(sizeof(GLint) * 12, sizeof(GLint) * 4, data);
		});
	}


	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		if (!areAssetsReady())
			return;

		for each (const auto & componentParam in components) {
			auto & board = *(GameBoard_Component*)componentParam[0];
			auto & score = *(GameScore_Component*)componentParam[1];

			// Update Rendering Data
			// Determine number of chars in score
			constexpr int decimalPlaces[8] = { 10000000,1000000,100000,10000,1000,100,10,1 };
			GLuint scoreLength = 1;
			for (GLuint x = 0; x < 8; ++x)
				if (score.m_score >= decimalPlaces[x]) {
					scoreLength = 8-x;
					break;
				}
			const GLuint doubledLength = scoreLength * 2u;
			m_bufferIndirectScore.write(sizeof(GLuint), sizeof(GLuint), &doubledLength);
			m_shaderScore->setUniform(4, scoreLength);
		
			// Generate sprite set for scored tiles
			const GLuint matchedCount = score.m_scoredTiles.size() ? (GLuint)(score.m_scoredTiles[0].first.size()) * 16u : 0u;
			m_bufferIndirectMatchedTiles.write(sizeof(GLuint), sizeof(GLuint), &matchedCount);
			struct ScoredStruct {
				glm::ivec4 coords;
				glm::vec2 center;
				glm::ivec2 count; 
				GLuint pieceStates[16];
			};
			unsigned long writeIndex = unsigned long(0);
			// Go through each set of scored tiles
			for (size_t n = 0; n < score.m_scoredTiles.size(); ++n) {
				// Find the center point in the set of scored tiles
				glm::ivec2 countAndType((int)score.m_scoredTiles[n].first.size(), board.m_tiles[score.m_scoredTiles[n].first[0].y][score.m_scoredTiles[n].first[0].x].m_type);
				glm::vec2 center(0.0f);
				for (size_t x = 0; x < score.m_scoredTiles[n].first.size(); ++x) 
					center += glm::vec2(score.m_scoredTiles[n].first[x].x, score.m_scoredTiles[n].first[x].y);
				center /= float(countAndType.x);
				// Go throuh a single set of scored tiles
				for (size_t x = 0; x < score.m_scoredTiles[n].first.size(); ++x) {
					const glm::ivec2 coords(score.m_scoredTiles[n].first[x].x, score.m_scoredTiles[n].first[x].y);
					m_bufferMatchedTiles.write(writeIndex, sizeof(glm::ivec2), &coords);
					writeIndex += sizeof(glm::ivec2);
					m_bufferMatchedTiles.write(writeIndex, sizeof(glm::vec2), &center);
					writeIndex += sizeof(glm::vec2);
					m_bufferMatchedTiles.write(writeIndex, sizeof(glm::ivec2), &countAndType);
					writeIndex += sizeof(glm::ivec2); // using '2' because we need to pad it
					GLuint states[16] = { 10,7,7,11, 4,16,16,6, 4,16,16,6, 8,5,5,9 };
					// If tile to the left
					if (score.m_scoredAdjacency[n][x].scored[1][0]) {
						states[0] = (score.m_scoredAdjacency[n][x].scored[0][1]) ? 3 : 7; // if beneath
						states[4] = 16;
						states[8] = 16;
						states[12] = (score.m_scoredAdjacency[n][x].scored[2][1]) ? 0 : 5; // if above
					}
					// If tile to the right
					if (score.m_scoredAdjacency[n][x].scored[1][2]) {
						states[3] = (score.m_scoredAdjacency[n][x].scored[0][1]) ? 2 : 7; // if beneath
						states[7] = 16;
						states[11] = 16;
						states[15] = (score.m_scoredAdjacency[n][x].scored[2][1]) ? 1 : 5; // if above
					}
					// If tile to the top
					if (score.m_scoredAdjacency[n][x].scored[2][1]) {
						states[12] = (score.m_scoredAdjacency[n][x].scored[1][0]) ? 0 : 4; // if left
						states[13] = 16;
						states[14] = 16;
						states[15] = (score.m_scoredAdjacency[n][x].scored[1][2]) ? 1 : 6; // if right
					}
					// If tile to the bottom
					if (score.m_scoredAdjacency[n][x].scored[0][1]) {
						states[0] = (score.m_scoredAdjacency[n][x].scored[1][0]) ? 3 : 4; // if left
						states[1] = 16;
						states[2] = 16;
						states[3] = (score.m_scoredAdjacency[n][x].scored[1][2]) ? 2 : 6; // if right
					}
					// Delete bottom left corner
					if (score.m_scoredAdjacency[n][x].scored[0][0] && score.m_scoredAdjacency[n][x].scored[0][1] && score.m_scoredAdjacency[n][x].scored[1][0])
						states[0] = 16;
					// Delete bottom right corner
					if (score.m_scoredAdjacency[n][x].scored[0][2] && score.m_scoredAdjacency[n][x].scored[0][1] && score.m_scoredAdjacency[n][x].scored[1][2])
						states[3] = 16;
					// Delete top left corner
					if (score.m_scoredAdjacency[n][x].scored[2][0] && score.m_scoredAdjacency[n][x].scored[2][1] && score.m_scoredAdjacency[n][x].scored[1][0])
						states[12] = 16;
					// Delete top right corner
					if (score.m_scoredAdjacency[n][x].scored[2][2] && score.m_scoredAdjacency[n][x].scored[2][1] && score.m_scoredAdjacency[n][x].scored[1][2])
						states[15] = 16;
					for (GLuint y = 0; y < 16u; ++y) {
						m_bufferMatchedTiles.write(writeIndex, sizeof(GLint), &states[y]);
						writeIndex += sizeof(GLint);
					}
				}
			}
			// Get scored tile count
			const GLuint scoreCount = (GLuint)score.m_scoredTiles.size();
			m_bufferIndirectMatchedNo.write(sizeof(GLuint), sizeof(GLuint), &scoreCount);

			// Render level XP to border FBO
			glViewport(0, 0, tileSize * 16, 1);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboIDBorder);
			glClear(GL_COLOR_BUFFER_BIT);
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			m_shaderBorder->bind();
			m_shaderBorder->setUniform(0, score.m_levelLinear);
			m_shaderBorder->setUniform(1, score.m_levelUpLinear);
			m_bufferIndirectBorder.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// Prepare center FBO
			glViewport(0, 0, tileSize * 6, tileSize * 12);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboIDField);
			glClear(GL_COLOR_BUFFER_BIT);

			// Render Matched tiles to the FBO
			m_shaderMatchedTiles->bind();
			m_shaderMatchedTiles->setUniform(0, m_orthoProjField);
			m_textureMatchedTiles->bind(0);
			m_bufferIndirectMatchedTiles.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			m_bufferMatchedTiles.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 9);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// Render game tiles to the FBO
			m_shaderTiles->bind();
			m_textureTile->bind(0);
			m_textureTilePlayer->bind(1);
			m_shaderTiles->setUniform(0, m_orthoProjField);
			glBindVertexArray(m_shapeQuad->m_vaoID);
			m_bufferIndirectTiles.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);			

			// Render Match count to the FBO
			m_shaderMatchedNo->bind();
			m_shaderMatchedNo->setUniform(0, m_orthoProjField);
			m_textureNums->bind(1);
			m_bufferIndirectMatchedNo.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// Render score header bar to the FBO
			glViewport(0, 0, tileSize * 6, tileSize * 1.5);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboIDBars);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glClear(GL_COLOR_BUFFER_BIT);
			m_shaderScore->bind();
			m_shaderScore->setUniform(0, m_orthoProjHeader);
			m_bufferIndirectScore.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
			
			// Render multiplier into header bar
			m_shaderMultiplier->bind();
			m_shaderMultiplier->setUniform(0, m_orthoProjHeader);
			m_bufferIndirectMultiplier.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);

			// Render time footer bar to the FBO			
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glClear(GL_COLOR_BUFFER_BIT);
			m_shaderTimer->bind();
			m_shaderTimer->setUniform(0, m_orthoProjHeader);
			m_textureTimeStop->bind(0);
			m_bufferIndirectStop.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
			
			// Render all of the textures onto the board model
			glViewport(0, 0, m_renderSize.x, m_renderSize.y);
			glBindFramebuffer(GL_FRAMEBUFFER, m_lightingFBOID);
			glBindVertexArray(*m_vaoModels);
			m_bufferIndirectBoard.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			m_shaderBoard->bind();
			glBindTextureUnit(0, m_borderTexID);
			glBindTextureUnit(1, m_boardTexID);
			glBindTextureUnit(2, m_scoreTexID);
			glBindTextureUnit(3, m_timeTexID);
			glMultiDrawArraysIndirect(GL_TRIANGLES, 0, 4, 0);

			// End
			glDisable(GL_BLEND);
		}
	}

	const bool areAssetsReady() const {
		return (
			m_shapeQuad->existsYet() &&
			m_modelBoard->existsYet() &&
			m_shaderTiles->existsYet() &&
			m_shaderMatchedTiles->existsYet() &&
			m_shaderBoard->existsYet() &&
			m_shaderScore->existsYet() &&
			m_shaderMultiplier->existsYet() &&
			m_shaderTimer->existsYet() &&
			m_textureTile->existsYet() &&
			m_textureMatchedTiles->existsYet() &&
			m_textureNums->existsYet() &&
			m_textureTimeStop->existsYet()
		);
	}


private:
	// Private Attributes
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	GLuint m_lightingFBOID = 0, m_fboIDBorder = 0, m_borderTexID = 0, m_fboIDField = 0, m_boardTexID = 0, m_fboIDBars = 0, m_scoreTexID = 0, m_timeTexID = 0;
	glm::ivec2 m_renderSize = glm::ivec2(1);	
	Shared_Asset_Primitive m_shapeQuad;
	Shared_Asset_Texture m_textureNums;

	// Board Rendering Resources
	Shared_Asset_Shader m_shaderBoard;
	Shared_Asset_Model m_modelBoard, m_modelField, m_modelHeader, m_modelFooter;
	StaticBuffer m_bufferIndirectBoard;
	const GLuint * m_vaoModels;

	// Border Rendering Resources
	Shared_Asset_Shader m_shaderBorder;
	StaticBuffer m_bufferIndirectBorder;

	// Tile Rendering Resources
	Shared_Asset_Shader m_shaderTiles;
	Shared_Asset_Texture m_textureTile, m_textureTilePlayer;
	StaticBuffer m_bufferIndirectTiles;
	glm::mat4 m_orthoProjField;

	// Matched Tiles Rendering Resources
	Shared_Asset_Shader m_shaderMatchedTiles, m_shaderMatchedNo;
	Shared_Asset_Texture m_textureMatchedTiles;
	StaticBuffer m_bufferIndirectMatchedTiles, m_bufferIndirectMatchedNo;
	DynamicBuffer m_bufferMatchedTiles;

	// Score Rendering Resources
	Shared_Asset_Shader m_shaderScore;
	Shared_Asset_Texture m_textureScoreNums;
	StaticBuffer m_bufferIndirectScore;
	glm::mat4 m_orthoProjHeader;

	// Multiplier Rendering Resources
	Shared_Asset_Shader m_shaderMultiplier;
	StaticBuffer m_bufferIndirectMultiplier;

	// Stop-Timer Rendering Resources
	Shared_Asset_Shader m_shaderTimer;
	Shared_Asset_Texture m_textureTimeStop;
	StaticBuffer m_bufferIndirectStop;

};

#endif // RENDERING_S_H