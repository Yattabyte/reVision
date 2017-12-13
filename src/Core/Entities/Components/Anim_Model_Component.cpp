#include "Entities\Components\Anim_Model_Component.h"
#include "Systems\World\ECSmessage.h"
#include "Systems\World\ECSmessages.h"
#include "Utilities\Frustum.h"
#include "Utilities\Transform.h"

Anim_Model_Component::~Anim_Model_Component()
{
	glDeleteBuffers(1, &m_uboID);
	glDeleteVertexArrays(1, &m_vao_id);
}

Anim_Model_Component::Anim_Model_Component(const ECShandle &id, const ECShandle &pid, Engine_Package *enginePackage) : Geometry_Component(id, pid)
{
	m_updateBuffers = false;
	m_uboID = 0;
	m_vao_id = 0;
	glGenBuffers(1, &m_uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Transform_Buffer), &m_uboData, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
	GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p, &m_uboData, sizeof(Transform_Buffer));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0); 
	
	glGenVertexArrays(1, &m_vao_id);
	glBindVertexArray(m_vao_id);
	for (unsigned int x = 0; x < 8; ++x)
		glEnableVertexAttribArray(x);
	glBindVertexArray(0);
}

void Anim_Model_Component::Update()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Transform_Buffer), &m_uboData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	if (m_updateBuffers)
		UpdateBuffers();
}

void Anim_Model_Component::UpdateBuffers()
{
	if (m_model) {
		shared_lock<shared_mutex> guard(m_model->m_mutex);
		auto &buffers = m_model->buffers;
		if (buffers[0] != GLuint(-1)) {
			glBindVertexArray(m_vao_id);

			glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
			glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, buffers[5]);
			glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, buffers[6]);
			glVertexAttribIPointer(6, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
			glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

			glBindVertexArray(0);
			m_updateBuffers = false;
		}
	}
}

void Anim_Model_Component::Draw()
{
	if (m_updateBuffers)
		UpdateBuffers();
	if (m_model) {
		shared_lock<shared_mutex> guard(m_model->m_mutex);

		glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
		glBindVertexArray(m_vao_id);
		glDrawArrays(GL_TRIANGLES, 0, m_model->mesh_size);
		glBindVertexArray(0);
	}
}

bool Anim_Model_Component::IsVisible(const mat4 & PVMatrix)
{
	if (m_model) {
		shared_lock<shared_mutex> guard(m_model->m_mutex);
		Frustum frustum(PVMatrix * m_uboData.mMatrix);

		if (frustum.AABBInFrustom(m_model->bbox_min, m_model->bbox_max))
			return true;
	}
	return false;	
}

void Anim_Model_Component::ReceiveMessage(const ECSmessage &message)
{
	if (Component::Am_I_The_Sender(message)) return;
	switch (message.GetCommandID())
	{
		case SET_MODEL_DIR: {
			if (!message.IsOfType<string>()) break;
			const auto &payload = message.GetPayload<string>();
			Asset_Manager::load_asset(m_model, payload);
			m_updateBuffers = true;
			break;
		}	
		case SET_MODEL_TRANSFORM: {
			if (!message.IsOfType<Transform>()) break;
			const auto &payload = message.GetPayload<Transform>();
			m_uboData.mMatrix = payload.modelMatrix;
			Update();
			break;
		}
	}
}
