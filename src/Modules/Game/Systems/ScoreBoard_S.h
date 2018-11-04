#pragma once
#ifndef SCOREBOARD_S_H
#define SCOREBOARD_S_H 

#include "Utilities\ECS\ecsSystem.h"
#include "Assets\Asset_Primitive.h"
#include "Assets\Asset_Shader.h"
#include "Assets\Asset_Texture.h"
#include "Utilities\GL\StaticBuffer.h"
#include "Engine.h"



/** A system that updates the rendering state for spot lighting, using the ECS system. */
class ScoreBoard_System : public BaseECSSystem {
public:
	// (de)Constructors
	~ScoreBoard_System() = default;
	ScoreBoard_System(Engine * engine) {
		// Declare component types used
		addComponentType(BoardState_Component::ID);

		// Asset Loading
		m_shaderScore = Asset_Shader::Create(engine, "Game\\Score", true);
		m_texture7Seg = Asset_Texture::Create(engine, "Game\\7segnums.png");
		m_shapeQuad = Asset_Primitive::Create(engine, "quad");

		// Asset-Finished Callbacks
		m_shapeQuad->addCallback(m_aliveIndicator, [&]() mutable {
			const GLuint data[4] = { (GLuint)m_shapeQuad->getSize(), 8, 0, 0 }; // count, primCount, first, reserved
			m_bufferIndirectScore = StaticBuffer(sizeof(GLuint) * 4, data, 0);
		});
	}



	// Interface Implementation	
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		if (!m_shapeQuad->existsYet() || !m_shaderScore->existsYet() || !m_texture7Seg->existsYet())
			return;

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		m_shaderScore->bind();
		m_texture7Seg->bind(0);
		glBindVertexArray(m_shapeQuad->m_vaoID);
		m_bufferIndirectScore.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
		for each (const auto & componentParam in components) {
			BoardState_Component & board = *(BoardState_Component*)componentParam[0];
			m_shaderScore->setUniform(0, board.m_animtedScore);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}

		glDisable(GL_BLEND);
	}


private:
	// Private Attributes
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
	Shared_Asset_Shader m_shaderScore;
	Shared_Asset_Primitive m_shapeQuad;
	Shared_Asset_Texture m_texture7Seg;
	StaticBuffer m_bufferIndirectScore;
};

#endif // SCOREBOARD_S_H