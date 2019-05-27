#pragma once
#ifndef RENDERING_S_H
#define RENDERING_S_H 

#include "States/GameSystemInterface.h"
#include "States/Puzzle/ECS/components.h"
#include "Assets/Primitive.h"
#include "Assets/Model.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"
#include "Utilities/GL/StaticBuffer.h"
#include "Utilities/GL/DynamicBuffer.h"
#include "Engine.h"


/** Responsible for rendering the game to the screen. */
class Rendering_System : public Game_System_Interface {
public:
	// Public (de)Constructors
	/** Destroy this puzzle rendering system. */
	inline ~Rendering_System() {
		// Update indicator
		m_aliveIndicator = false;
	}
	/** Construct a puzzle rendering system. */
	inline Rendering_System(Engine * engine, const GLuint & lightingFBOID) : m_lightingFBOID(lightingFBOID) {
		// Declare component types used
		addComponentType(Score_Component::ID);
		
		m_vaoModels = &engine->getManager_Models().getVAO();

		// For rendering tiles to the board
		m_orthoProjField = glm::ortho<float>(0, TILE_SIZE * 6, 0, TILE_SIZE * 12, -1, 1);
		m_orthoProjHeader = glm::ortho<float>(-384, 384, -96, 96, -1, 1);

		// FBO & Texture Creation
		glCreateFramebuffers(1, &m_fboIDBorder);
		glCreateTextures(GL_TEXTURE_1D, 1, &m_borderTexID);
		glTextureImage1DEXT(m_borderTexID, GL_TEXTURE_1D, 0, GL_RGBA16F, GLsizei(TILE_SIZE * 16), 0, GL_RGBA, GL_FLOAT, NULL);
		glTextureParameteri(m_borderTexID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_borderTexID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_borderTexID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameterf(m_borderTexID, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
		glNamedFramebufferTexture(m_fboIDBorder, GL_COLOR_ATTACHMENT0, m_borderTexID, 0);
		glNamedFramebufferDrawBuffer(m_fboIDBorder, GL_COLOR_ATTACHMENT0);

		glCreateFramebuffers(1, &m_fboIDField);
		glCreateTextures(GL_TEXTURE_2D, 1, &m_boardTexID);
		glTextureImage2DEXT(m_boardTexID, GL_TEXTURE_2D, 0, GL_RGBA16F, GLsizei(TILE_SIZE * 6), GLsizei(TILE_SIZE * 12), 0, GL_RGBA, GL_FLOAT, NULL);
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
		glTextureImage2DEXT(m_scoreTexID, GL_TEXTURE_2D, 0, GL_RGBA16F, GLsizei(TILE_SIZE * 6), GLsizei(TILE_SIZE * 1.5), 0, GL_RGBA, GL_FLOAT, NULL);
		glTextureImage2DEXT(m_timeTexID, GL_TEXTURE_2D, 0, GL_RGBA16F, GLsizei(TILE_SIZE * 6), GLsizei(TILE_SIZE * 1.5), 0, GL_RGBA, GL_FLOAT, NULL);
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
		auto & msgManager = engine->getManager_Messages();
		if (glCheckNamedFramebufferStatus(m_fboIDField, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			msgManager.error("Game Board Framebuffer has encountered an error.");
		if (glCheckNamedFramebufferStatus(m_fboIDBars, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			msgManager.error("Game Header Framebuffer has encountered an error.");
		if (!glIsTexture(m_scoreTexID))
			msgManager.error("Game Header Texture is incomplete.");
		if (!glIsTexture(m_borderTexID))
			msgManager.error("Game Border Texture is incomplete.");
		if (!glIsTexture(m_boardTexID))
			msgManager.error("Game Board Texture is incomplete.");
		if (!glIsTexture(m_timeTexID))
			msgManager.error("Game Score Texture is incomplete.");

		// Asset Loading
		m_modelBoard = Shared_Model(engine, "Game\\boardBorder.obj");
		m_modelField = Shared_Model(engine, "Game\\boardField.obj");
		m_modelHeader = Shared_Model(engine, "Game\\boardTop.obj");
		m_modelFooter = Shared_Model(engine, "Game\\boardBottom.obj");
		m_shaderBorder = Shared_Shader(engine, "Game\\Border");
		m_shaderBoard = Shared_Shader(engine, "Game\\Board");
		m_shaderTiles = Shared_Shader(engine, "Game\\Tiles");
		m_shaderMatchedTiles = Shared_Shader(engine, "Game\\Matched");
		m_shaderMatchedNo = Shared_Shader(engine, "Game\\MatchedNumber");
		m_shaderBackground = Shared_Shader(engine, "Game\\Background");
		m_shaderIntro = Shared_Shader(engine, "Game\\Intro");
		m_shaderScore = Shared_Shader(engine, "Game\\Score");
		m_shaderMultiplier = Shared_Shader(engine, "Game\\Multiplier");
		m_shaderTimer = Shared_Shader(engine, "Game\\Timer");
		m_textureTile = Shared_Texture(engine, "Game\\tile.png");
		m_textureCountdown = Shared_Texture(engine, "Game\\countdown.png");
		m_textureTilePlayer = Shared_Texture(engine, "Game\\player.png");
		m_textureMatchedTiles = Shared_Texture(engine, "Game\\scorePieces.png");
		m_textureNums = Shared_Texture(engine, "Game\\numbers.png");
		m_textureTimeStop = Shared_Texture(engine, "Game\\timestop.png");
		m_shapeQuad = Shared_Primitive(engine, "quad");

		// Preferences
		auto & preferences = engine->getPreferenceState();
		preferences.getOrSetValue(PreferenceState::C_WINDOW_WIDTH, m_renderSize.x);
		preferences.addCallback(PreferenceState::C_WINDOW_WIDTH, m_aliveIndicator, [&](const float &f) {m_renderSize.x = int(f); });
		preferences.getOrSetValue(PreferenceState::C_WINDOW_HEIGHT, m_renderSize.y);
		preferences.addCallback(PreferenceState::C_WINDOW_HEIGHT, m_aliveIndicator, [&](const float &f) {m_renderSize.y = int(f); });

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint quadSize = (GLuint)m_shapeQuad->getSize();
			const GLsizeiptr bufferSize = sizeof(GLuint) * 4;
			/* count, primCount, first, reserved */
			const GLuint quad1[4] = { quadSize, 1, 0, 0 };
			m_indirectBorder		= StaticBuffer(bufferSize, quad1, 0);
			m_indirectBackground	= StaticBuffer(bufferSize, quad1, 0);
			m_indirectIntro			= StaticBuffer(bufferSize, quad1, 0);
			m_indirectScore			= StaticBuffer(bufferSize, quad1);

			const GLuint quadAllTiles[4] = { quadSize, (12 * 6) + 2, 0, 0 };
			m_indirectTiles			= StaticBuffer(bufferSize, quadAllTiles, 0);
			
			const GLuint quad0[4] = { quadSize, 0, 0, 0 };
			m_indirectMatchedNo		= StaticBuffer(bufferSize, quad0, GL_DYNAMIC_STORAGE_BIT);
			m_indirectMatchedTiles	= StaticBuffer(bufferSize, quad0, GL_DYNAMIC_STORAGE_BIT);

			const GLuint quad6[4] = { quadSize, 6, 0, 0 };
			m_indirectStop			= StaticBuffer(bufferSize, quad6, 0);

			const GLuint quad4[4] = { quadSize, 4, 0, 0 };
			m_indirectMultiplier	= StaticBuffer(bufferSize, quad4, 0);
		});
		m_indirectBoard = StaticBuffer(sizeof(GLint) * 16, 0, GL_DYNAMIC_STORAGE_BIT);
		m_modelBoard->addCallback(m_aliveIndicator, [&]() mutable {
			const GLint data[4] = { GLint(m_modelBoard->m_count), 1, GLint(m_modelBoard->m_offset), 1 };
			m_indirectBoard.write(0, sizeof(GLint) * 4, data);
		});
		m_modelField->addCallback(m_aliveIndicator, [&]() mutable {
			const GLint data[4] = { GLint(m_modelField->m_count), 1, GLint(m_modelField->m_offset), 1 };
			m_indirectBoard.write(sizeof(GLint) * 4, sizeof(GLint) * 4, data);
		});
		m_modelHeader->addCallback(m_aliveIndicator, [&]() mutable {
			const GLint data[4] = { GLint(m_modelHeader->m_count), 1, GLint(m_modelHeader->m_offset), 1 };
			m_indirectBoard.write(sizeof(GLint) * 8, sizeof(GLint) * 4, data);
		});
		m_modelFooter->addCallback(m_aliveIndicator, [&]() mutable {
			const GLint data[4] = { GLint(m_modelFooter->m_count), 1,GLint(m_modelFooter->m_offset), 1 };
			m_indirectBoard.write(sizeof(GLint) * 12, sizeof(GLint) * 4, data);
		});
	}


	// Public Interface Implementation
	inline virtual bool readyToUse() override {
		return 
			m_modelBoard->existsYet() &&
			m_modelField->existsYet() &&
			m_modelHeader->existsYet() &&
			m_modelFooter->existsYet() &&
			m_shapeQuad->existsYet() &&
			m_shaderBackground->existsYet() &&
			m_shaderBorder->existsYet() &&
			m_shaderBoard->existsYet() &&
			m_shaderTiles->existsYet() &&
			m_shaderMatchedTiles->existsYet() &&
			m_shaderMatchedNo->existsYet() &&
			m_shaderScore->existsYet() &&
			m_shaderMultiplier->existsYet() &&
			m_shaderTimer->existsYet() &&
			m_textureTile->existsYet() &&
			m_textureTilePlayer->existsYet() &&
			m_textureMatchedTiles->existsYet() &&
			m_textureNums->existsYet() &&
			m_textureTimeStop->existsYet();
	}
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			// Update elements
			update(*(Score_Component*)componentParam[0]);
			
			// Render elements	
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			renderBorder();
			renderField();
			renderIntro();
			renderHeader();
			renderFooter();
			renderBackground();
			renderBoard();
			glDisable(GL_BLEND);
		}
	}


private:
	// Private Logic Functions
	/** Update Rendering Data. */
	inline void update(const Score_Component & score) {
		// System Time
		score.m_data->data->sysTime = float(glfwGetTime());

		// Determine number of chars in score
		constexpr int decimalPlaces[8] = { 10000000,1000000,100000,10000,1000,100,10,1 };
		GLuint scoreLength = 1;
		for (GLuint x = 0; x < 8; ++x)
			if (score.m_score >= decimalPlaces[x]) {
				scoreLength = 8 - x;
				break;
			}
		const GLuint doubledLength = scoreLength * 2u;
		m_indirectScore.write(sizeof(GLuint), sizeof(GLuint), &doubledLength);
		m_shaderScore->setUniform(4, scoreLength);

		// Generate sprite set for scored tiles
		GLuint matchedCount = 0;
		for each (const auto & qwe in score.m_scoredTiles)
			matchedCount += (GLuint(qwe.xy.size()) * 16u);
		m_indirectMatchedTiles.write(sizeof(GLuint), sizeof(GLuint), &matchedCount);
		unsigned long writeIndex = unsigned long(0);
		// Go through each set of scored tiles
		for (size_t n = 0; n < score.m_scoredTiles.size(); ++n) {
			// Go throuh a single set of scored tiles
			for (size_t x = 0; x < score.m_scoredTiles[n].xy.size(); ++x) {
				const glm::ivec2 coords(score.m_scoredTiles[n].xy[x].x, score.m_scoredTiles[n].xy[x].y);
				m_bufferMatchedTiles.write(writeIndex, sizeof(glm::ivec2), &coords);
				writeIndex += sizeof(glm::ivec2);
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
		writeIndex = unsigned long(0);
		for (size_t n = 0; n < score.m_scoredTiles.size(); ++n) {
			glm::vec2 center(0.0f);
			glm::ivec2 countAndType((int)score.m_scoredTiles[n].xy.size(), 0);

			// Find the center point in the set of scored tiles
			int commonX[6] = { 0,0,0,0,0,0 };
			int commonY[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
			for (size_t a = 0; a < score.m_scoredTiles[n].xy.size(); ++a) {
				commonX[score.m_scoredTiles[n].xy[a].x]++;
				commonY[score.m_scoredTiles[n].xy[a].y]++;
			}
			// Find most common X
			int largest = 0;
			int mostCommon = -1;
			for (int a = 0; a < 6; ++a) 
				if (commonX[a] > largest) {
					largest = commonX[a];
					mostCommon = a;
				}
			if (largest > 1 && mostCommon != -1)
				center.x = float(mostCommon);
			else {
				// Calculate average x instead
				for (size_t a = 0; a < score.m_scoredTiles[n].xy.size(); ++a)
					center.x += score.m_scoredTiles[n].xy[a].x;
				center.x /= float(countAndType.x);
			}
			// Find most common Y
			largest = 0;
			mostCommon = -1;
			for (int a = 0; a < 12; ++a) 
				if (commonY[a] > largest) {
					largest = commonY[a];
					mostCommon = a;
				}			
			if (largest > 1 && mostCommon != -1) 
				center.y = float(mostCommon);			
			else {
				// Calculate average y instead
				for (size_t a = 0; a < score.m_scoredTiles[n].xy.size(); ++a)
					center.y += score.m_scoredTiles[n].xy[a].y;
				center.y /= float(countAndType.x);
			}		
			
			m_bufferMatchedNos.write(writeIndex, sizeof(glm::vec2), &center);
			writeIndex += sizeof(glm::vec2);
			m_bufferMatchedNos.write(writeIndex, sizeof(glm::ivec2), &countAndType);
			writeIndex += sizeof(glm::ivec2); // using '2' because we need to pad it					
		}
		const GLuint scoreCount = (GLuint)score.m_scoredTiles.size();
		m_indirectMatchedNo.write(sizeof(GLuint), sizeof(GLuint), &scoreCount);

		m_shaderBorder->setUniform(0, score.m_levelLinear);
		m_shaderBorder->setUniform(1, score.m_levelUpLinear);
	}
	// Private Rendering Functions
	/** Render game level / border into it's own FBO. */
	inline void renderBorder() {
		glViewport(0, 0, TILE_SIZE * 16, 1);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboIDBorder);
		glClear(GL_COLOR_BUFFER_BIT);
		m_shaderBorder->bind();
		m_indirectBorder.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
	/** Render game tiles into it's own FBO. */
	inline void renderField() {
		// Prepare center FBO
		glViewport(0, 0, TILE_SIZE * 6, TILE_SIZE * 12);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboIDField);
		glClear(GL_COLOR_BUFFER_BIT);

		// Matched tiles first, behind everything
		m_shaderMatchedTiles->bind();
		m_shaderMatchedTiles->setUniform(0, m_orthoProjField);
		m_textureMatchedTiles->bind(0);
		m_indirectMatchedTiles.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_bufferMatchedTiles.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 9);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Game tiles
		m_shaderTiles->bind();
		m_textureTile->bind(0);
		m_textureTilePlayer->bind(1);
		m_shaderTiles->setUniform(0, m_orthoProjField);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_indirectTiles.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Match count
		m_shaderMatchedNo->bind();
		m_shaderMatchedNo->setUniform(0, m_orthoProjField);
		m_textureNums->bind(1);
		m_indirectMatchedNo.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_bufferMatchedNos.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 9);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
	/** Render intro countdown into game field FBO. */
	inline void renderIntro() {
		// Intro
		m_shaderIntro->bind();
		m_textureCountdown->bind(0);
		m_indirectIntro.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
	/** Render score header bar into it's own FBO. */
	inline void renderHeader() {
		// Score
		glViewport(0, 0, GLsizei(TILE_SIZE * 6), GLsizei(TILE_SIZE * 1.5));
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboIDBars);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glClear(GL_COLOR_BUFFER_BIT);
		m_shaderScore->bind();
		m_shaderScore->setUniform(0, m_orthoProjHeader);
		m_indirectScore.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);

		// Multiplier
		m_shaderMultiplier->bind();
		m_shaderMultiplier->setUniform(0, m_orthoProjHeader);
		m_indirectMultiplier.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
	/** Render time footer bar into it's own FBO. */
	inline void renderFooter() {
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glClear(GL_COLOR_BUFFER_BIT);
		m_shaderTimer->bind();
		m_shaderTimer->setUniform(0, m_orthoProjHeader);
		m_textureTimeStop->bind(0);
		m_indirectStop.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
	}
	/** Render background effect to the screen*/
	inline void renderBackground() {
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glViewport(0, 0, m_renderSize.x, m_renderSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, m_lightingFBOID);
		m_shaderBackground->bind();
		m_indirectBackground.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		glDrawArraysIndirect(GL_TRIANGLES, 0);
		glEnable(GL_BLEND);
	}
	/** Render the board model with all the previous textures onto the screen. */
	inline void renderBoard() {			
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(*m_vaoModels);
		m_indirectBoard.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		m_shaderBoard->bind();
		glBindTextureUnit(0, m_borderTexID);
		glBindTextureUnit(1, m_boardTexID);
		glBindTextureUnit(2, m_scoreTexID);
		glBindTextureUnit(3, m_timeTexID);
		glMultiDrawArraysIndirect(GL_TRIANGLES, 0, 4, 0);
		glDisable(GL_DEPTH_TEST);
	}
	

	// Private Attributes
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	GLuint m_lightingFBOID = 0, m_fboIDBorder = 0, m_borderTexID = 0, m_fboIDField = 0, m_boardTexID = 0, m_fboIDBars = 0, m_scoreTexID = 0, m_timeTexID = 0;
	glm::ivec2 m_renderSize = glm::ivec2(1);	
	Shared_Primitive m_shapeQuad;
	Shared_Texture m_textureNums;

	// Background
	Shared_Shader m_shaderBackground;
	StaticBuffer m_indirectBackground;

	// Board
	Shared_Shader m_shaderBoard;
	Shared_Model m_modelBoard, m_modelField, m_modelHeader, m_modelFooter;
	StaticBuffer m_indirectBoard;
	const GLuint * m_vaoModels;

	// Border
	Shared_Shader m_shaderBorder;
	StaticBuffer m_indirectBorder;

	// Tile
	Shared_Shader m_shaderTiles;
	Shared_Texture m_textureTile, m_textureTilePlayer;
	StaticBuffer m_indirectTiles;
	glm::mat4 m_orthoProjField;

	// Matched Tiles
	Shared_Shader m_shaderMatchedTiles, m_shaderMatchedNo;
	Shared_Texture m_textureMatchedTiles;
	StaticBuffer m_indirectMatchedTiles, m_indirectMatchedNo;
	DynamicBuffer m_bufferMatchedTiles, m_bufferMatchedNos;

	// Intro-Outro
	Shared_Shader m_shaderIntro;
	Shared_Texture m_textureCountdown;
	StaticBuffer m_indirectIntro;

	// Score
	Shared_Shader m_shaderScore;
	StaticBuffer m_indirectScore;
	glm::mat4 m_orthoProjHeader;

	// Multiplier
	Shared_Shader m_shaderMultiplier;
	StaticBuffer m_indirectMultiplier;

	// Stop-Timer
	Shared_Shader m_shaderTimer;
	Shared_Texture m_textureTimeStop;
	StaticBuffer m_indirectStop;

};

#endif // RENDERING_S_H