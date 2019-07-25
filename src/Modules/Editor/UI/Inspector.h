#pragma once
#ifndef INSPECTOR_H
#define INSPECTOR_H

#include "Modules/UI/UI_M.h"


// Forward declarations
class Engine;
class LevelEditor_Module;

/***/
class Inspector : public ImGUI_Element {
public:
	// Public (de)Constructors
	/***/
	inline ~Inspector() = default;
	/***/
	Inspector(Engine * engine, LevelEditor_Module * editor);


	// Public Interface Implementation
	virtual void render(const float & deltaTime) override;


private:
	// Private Attributes
	LevelEditor_Module * m_editor = nullptr;
	glm::ivec2 m_renderSize = glm::ivec2(1);
	std::shared_ptr<bool> m_aliveIndicator = std::make_shared<bool>(true);
};

#endif // INSPECTOR_H