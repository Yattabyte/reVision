#pragma once
#ifndef GAME_MODULE_H
#define GAME_MODULE_H

#include "Modules\Engine_Module.h"
#include "Modules\Game\Components\GameBoard_C.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Shader.h"
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
	float m_timeAccumulator = 0.0f;
	ECSSystemList m_gameplaySystems;
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	GLuint m_fboID = 0, m_boardTexID = 0;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	VectorBuffer<BoardBuffer> m_boardBuffer;
	Shared_Asset_Primitive m_shapeQuad;


	// Board Rendering Resources
	Shared_Asset_Shader m_shaderTiles, m_shaderBoard;
	Shared_Asset_Texture m_textureTile, m_texturePlayer;
	StaticBuffer m_bufferIndirectTiles, m_bufferIndirectBoard;

	Shared_Asset_Shader m_shaderScore;
	Shared_Asset_Texture m_texture7Seg;
	StaticBuffer m_bufferIndirectScore;

};

#endif // GAME_MODULE_H