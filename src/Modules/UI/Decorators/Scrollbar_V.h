#pragma once
#ifndef UI_SCROLLBAR_V_H
#define UI_SCROLLBAR_V_H

#include "Modules/UI/Basic Elements/Button.h"
#include "Modules/UI/Decorators/UI_Decorator.h"
#include <memory>
#include <string>


/** Scrollbar decorator object*/
class Scrollbar_V : public UI_Decorator
{
public:
	// Interaction enums
	enum interact {
		on_scroll_change = UI_Element::last_interact_index
	};


	// Public (de)Constructors
	~Scrollbar_V() {
		// Delete geometry
		glDeleteBuffers(1, &m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	Scrollbar_V(Engine * engine, const std::shared_ptr<UI_Element> & component) : UI_Decorator(component) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\ScrollBar");

		auto & topButton = std::make_shared<Button>(engine, ""), &bottomButton = std::make_shared<Button>(engine, ""), &panel = std::make_shared<Button>(engine, "");
		addElement(topButton);
		addElement(bottomButton);
		addElement(panel);
		topButton->setBevelRadius(0.0F);
		topButton->setMinScale(glm::vec2(12.5f));
		topButton->setMaxScale(glm::vec2(12.5f));
		bottomButton->setBevelRadius(0.0F);
		bottomButton->setMinScale(glm::vec2(12.5f));
		bottomButton->setMaxScale(glm::vec2(12.5f));
		panel->setMinScale(glm::vec2(12.5f, 12.5f));
		panel->setMaxScale(glm::vec2(12.5f, m_scale.y - 25.0f - 25.0f));
		setMinScale(glm::vec2(12.5f, getMinScale().y));

		// Generate vertex array
		glCreateVertexArrays(1, &m_vaoID);
		glEnableVertexArrayAttrib(m_vaoID, 0);
		glVertexArrayAttribBinding(m_vaoID, 0, 0);
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glCreateBuffers(1, &m_vboID);
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID, 0, sizeof(glm::vec3));
		constexpr auto num_data = 2 * 3;
		glNamedBufferStorage(m_vboID, num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		const GLuint quad[4] = { (GLuint)num_data, 1, 0, 0 };
		m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_CLIENT_STORAGE_BIT);
	}


	// Interface Implementation
	virtual void update() override {
		constexpr auto num_data = 2 * 3;
		std::vector<glm::vec3> m_data(num_data);

		// Background
		m_data[0] = { -1, -1, 0 };
		m_data[1] = { 1, -1, 0 };
		m_data[2] = { 1,  1, 0 };
		m_data[3] = { 1,  1, 0 };
		m_data[4] = { -1,  1, 0 };
		m_data[5] = { -1, -1, 0 };
		for (int x = 0; x < 6; ++x)
			m_data[x] = glm::vec3(m_scale.x - 12.5f, 0, 0) + (m_data[x] * glm::vec3(12.5f, m_scale.y, 0.0f));		

		glNamedBufferSubData(m_vboID, 0, num_data * sizeof(glm::vec3), &m_data[0]);

		updateElements();
		UI_Element::update();
		m_component->setPosition(glm::vec2(-12.5f, 0));
		m_component->setScale(glm::vec2(m_scale.x - 12.5f, m_scale.y));
	}
	virtual void mouseMove(const MouseEvent & mouseEvent) override {
		if (!getVisible() || !getEnabled()) return;
		if (mouseWithin(mouseEvent) || doElementsExceedBounds(m_scale)) {
			MouseEvent subEvent = mouseEvent;
			subEvent.m_xPos = mouseEvent.m_xPos - m_position.x;
			subEvent.m_yPos = mouseEvent.m_yPos - m_position.y;
			for each (auto & child in m_children) 
				child->mouseMove(subEvent);
			m_component->mouseMove(subEvent);
			if (m_children.size() == 3) {
				if (std::dynamic_pointer_cast<Button>(m_children[2])->getPressed()) {
					setLinear(float(subEvent.m_yPos) / (m_scale.y - 25.0f - 12.5f));
					updateElements();
				}
			}			
			enactCallback(on_mouse_enter);
		}
		else
			enactCallback(on_mouse_exit);
	}
	virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		const auto newPosition = position + m_position;
		const auto newScale = glm::min(m_scale, scale);
		if (m_shader->existsYet()) {
			m_shader->bind();
			m_shader->setUniform(0, glm::vec3(newPosition, m_depth));
			glBindVertexArray(m_vaoID);
			m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}
		UI_Decorator::renderElement(deltaTime, position, newScale);
	}


	// Public Methods
	/** Set the linear amount for the location of the scroll bar.
	@param	linear		the linear amount to put the scroll bar. */
	void setLinear(const float & linear) {
		m_linear = std::clamp<float>(linear, -1.0f, 1.0f);
		enactCallback(on_scroll_change);
	}
	/** Get the linear value for this scrollbar. 
	@return				the linear value for this scroll bar. */
	float getLinear() const {
		return m_linear;
	}


protected:
	// Protected Methods
	/** Update the position of all scrollbar elements. */
	void updateElements() {
		if (m_children.size() == 3) {
			// Buttons
			m_children[0]->setPosition(glm::vec2(getScale() - 12.5f));
			m_children[1]->setPosition(glm::vec2(getScale().x - 12.5f, -getScale().y + 12.5f));

			// Panel
			m_children[2]->setMaxScale(glm::vec2(12.5f, m_scale.y - 25.0f - 12.5));
			m_children[2]->setPosition(glm::vec2(getScale().x - 12.5f, m_linear * (m_scale.y - 25.0f - 12.5f)));
		}
	}


	// Protected Attributes
	float m_linear = 1.0f;


private:
	// Private Attributes
	GLuint m_vaoID = 0, m_vboID = 0;
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
};

#endif // UI_SCROLLBAR_V_H