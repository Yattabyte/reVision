#include "Entities\Components\Anim_Model_Component.h"

Anim_Model_Component::~Anim_Model_Component()
{
	glDeleteBuffers(1, &m_uboID);
}

Anim_Model_Component::Anim_Model_Component(const string & relativePath, Transform *worldState)
{
	Asset_Manager::load_asset(m_model, relativePath);
	m_transformData = worldState;
	glGenBuffers(1, &m_uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferData(GL_UNIFORM_BUFFER, (sizeof(mat4) * 101) + sizeof(float), &m_uboData, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
	GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p, &m_uboData, sizeof(Transform_Buffer));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Anim_Model_Component::Update()
{
	m_uboData.mMatrix = m_transformData->modelMatrix;
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(mat4x4), &m_transformData->modelMatrix);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Anim_Model_Component::Draw()
{
	shared_lock<shared_mutex> guard(m_model->m_mutex);

	glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
	glBindVertexArray(m_model->gl_vao_ID);
	glDrawArrays(GL_TRIANGLES, 0, m_model->mesh_size);
	glBindVertexArray(0);
}
