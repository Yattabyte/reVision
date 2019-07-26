#pragma once
#ifndef TITLEBAR_H
#define TITLEBAR_H

#include "Modules/UI/UI_M.h"


// Forward declarations
class Engine;
class LevelEditor_Module;

/***/
class TitleBar : public ImGUI_Element {
public:
	// Public (de)Constructors
	/***/
	inline ~TitleBar() = default;
	/***/
	TitleBar(Engine * engine, LevelEditor_Module * editor);


	// Public Interface Implementation
	virtual void tick(const float & deltaTime) override;


private:
	// Private Attributes
	LevelEditor_Module * m_editor = nullptr;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // TITLEBAR_H