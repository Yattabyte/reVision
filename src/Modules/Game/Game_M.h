#pragma once
#ifndef GAME_MODULE_H
#define GAME_MODULE_H

#include "Modules\Engine_Module.h"
#include "Modules\Game\Components\BoardState_C.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Texture.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\ECS\ECS.h"


/** A module responsible for the game. */
class Game_Module : public Engine_Module {
public:
	// (de)Constructors
	~Game_Module() = default;
	Game_Module() = default;


	// Public Interface Implementation
	virtual void initialize(Engine * engine) override;


	// Public Methods
	/** Increments the game simulation by a single tick. 
	@param		deltaTime		the delta time. */
	void tickGame(const float & deltaTime);


private:
	// Private Attributes
	ECSSystemList m_gameplaySystems;
	Shared_Asset_Shader m_shaderTiles, m_shaderBoard;
	Shared_Asset_Primitive m_shapeQuad;
	Shared_Asset_Texture m_textureTile, m_texturePlayer;
	StaticBuffer m_quad_tiles_indirect, m_quad_board_indirect;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	GLuint m_fboID = 0, m_boardTexID = 0;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	VectorBuffer<BoardBuffer> m_boardBuffer;
};

#endif // GAME_MODULE_H