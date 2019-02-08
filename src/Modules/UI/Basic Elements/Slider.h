#pragma once
#ifndef UI_SLIDER_H
#define UI_SLIDER_H

#include "Modules/UI/Basic Elements/UI_Element.h"
#include "Assets/Shader.h"
#include "Utilities/GL/StaticBuffer.h"


/** Slider ui element. */
class Slider : public UI_Element
{
public:
	// Interaction enums
	enum interact {
		on_slider_change = UI_Element::last_interact_index
	};


	// Public (de)Constructors
	~Slider() {
		// Delete geometry
		glDeleteBuffers(2, m_vboID);
		glDeleteVertexArrays(1, &m_vaoID);
	}
	Slider(Engine * engine) {
		// Asset Loading
		m_shader = Shared_Shader(engine, "UI\\Slider");

		// Generate vertex array
		glCreateVertexArrays(1, &m_vaoID);
		glEnableVertexArrayAttrib(m_vaoID, 0);
		glEnableVertexArrayAttrib(m_vaoID, 1);
		glVertexArrayAttribBinding(m_vaoID, 0, 0);
		glVertexArrayAttribBinding(m_vaoID, 1, 1);
		glVertexArrayAttribFormat(m_vaoID, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribIFormat(m_vaoID, 1, 1, GL_INT, 0);
		glCreateBuffers(2, m_vboID);
		glVertexArrayVertexBuffer(m_vaoID, 0, m_vboID[0], 0, sizeof(glm::vec3));
		glVertexArrayVertexBuffer(m_vaoID, 1, m_vboID[1], 0, sizeof(int));
		constexpr auto num_data = 6 * 3;
		glNamedBufferStorage(m_vboID[0], num_data * sizeof(glm::vec3), 0, GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferStorage(m_vboID[1], num_data * sizeof(int), 0, GL_DYNAMIC_STORAGE_BIT);
		const GLuint quad[4] = { (GLuint)num_data, 1, 0, 0 };
		m_indirect = StaticBuffer(sizeof(GLuint) * 4, quad, GL_CLIENT_STORAGE_BIT);
	}


	// Interface Implementation
	virtual void update() override {
		constexpr auto num_data = 6 * 3;
		std::vector<glm::vec3> data(num_data);
		std::vector<int> objIndices(num_data);

		for (int x = 0; x < 18; x += 6) {
			data[x + 0] = { -1, -1, 0 };
			data[x + 1] = { 1, -1, 0 };
			data[x + 2] = { 1,  1, 0 };
			data[x + 3] = { 1,  1, 0 };
			data[x + 4] = { -1,  1, 0 };
			data[x + 5] = { -1, -1, 0 };
		}

		for (int x = 0; x < 6; ++x) {
			data[x] *= glm::vec3(m_scale, 0);
			objIndices[x] = 0;
		}
		for (int x = 6; x < 12; ++x) {
			data[x] = (data[x] * glm::vec3((m_scale.x * m_percentage), m_scale.y, 0)) + glm::vec3((m_scale.x * m_percentage) - m_scale.x, 0, 0);
			objIndices[x] = 1;
		}
		for (int x = 12; x < 18; ++x) {
			data[x] = (data[x] * glm::vec3(5.0f, m_scale.y, 0)) + glm::vec3((2.0f * m_percentage - 1.0f) * (m_scale.x - 5.0f), 0, 0);
			objIndices[x] = 2;
		}
		
		glNamedBufferSubData(m_vboID[0], 0, num_data * sizeof(glm::vec3), &data[0]);
		glNamedBufferSubData(m_vboID[1], 0, num_data * sizeof(int), &objIndices[0]);

		UI_Element::update();
	}
	virtual void mouseMove(const MouseEvent & mouseEvent) override {
		if (!getVisible() || !getEnabled()) return;
		if (mouseWithin(mouseEvent) || doElementsExceedBounds(m_scale)) {
			m_highlighted = true;
			if (mouseEvent.m_action == GLFW_PRESS) {
				const float mx = float(mouseEvent.m_xPos) - m_position.x + m_scale.x;
				setPercentage(mx / (m_scale.x * 2.0f));
				enactCallback(on_slider_change);
			}
		}
		else
			m_highlighted = false;
		UI_Element::mouseMove(mouseEvent);
	}
	virtual bool mouseButton(const MouseEvent & mouseEvent) override {
		if (!getVisible() || !getEnabled()) return false;
		if (mouseWithin(mouseEvent) || doElementsExceedBounds(m_scale)) 
			m_pressed = mouseEvent.m_action == GLFW_PRESS;		
		else
			m_pressed = false;
		return UI_Element::mouseButton(mouseEvent);
	}
	virtual void renderElement(const float & deltaTime, const glm::vec2 & position, const glm::vec2 & scale) override {
		if (!getVisible()) return;
		const glm::vec2 newPosition = position + m_position;
		const glm::vec2 newScale = glm::min(m_scale, scale);
		if (m_shader->existsYet()) {			
			m_shader->bind();
			m_shader->setUniform(0, glm::vec3(newPosition, m_depth));
			m_shader->setUniform(1, m_enabled);
			m_shader->setUniform(2, m_highlighted);
			m_shader->setUniform(3, m_pressed);
			glBindVertexArray(m_vaoID);
			m_indirect.bindBuffer(GL_DRAW_INDIRECT_BUFFER);
			glDrawArraysIndirect(GL_TRIANGLES, 0);
		}
		UI_Element::renderElement(deltaTime, position, newScale);
	}


	// Public Methods
	/** Set the percentage for this slider.
	@param	percentage	the percentage amount to put this slider at. */
	void setPercentage(const float & linear) {
		m_percentage = std::clamp<float>(linear, -1.0f, 1.0f);
		update();
	}
	/** Get the percentage value for this scrollbar.
	@return				the percentage value for this slider. */
	float getPercentage() const {
		return m_percentage;
	}


protected:
	// Protected Attributes
	float m_percentage = 0.5f;
	bool 
		m_highlighted = false,
		m_pressed = false;


private:
	// Private Attributes
	GLuint 
		m_vaoID = 0, 
		m_vboID[2] = { 0, 0 };
	Shared_Shader m_shader;
	StaticBuffer m_indirect;
};

#endif // UI_SLIDER_H