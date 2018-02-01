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
	m_uboID = 0;
	m_vao_id = 0;
	m_skin = 0;
	m_fence = nullptr;
	glGenBuffers(1, &m_uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Transform_Buffer), &m_uboData, GL_DYNAMIC_COPY);
	GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p, &m_uboData, sizeof(Transform_Buffer));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0); 
	
	m_vao_id = Asset_Model::GenerateVAO();
}

void Anim_Model_Component::Update()
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Transform_Buffer), &m_uboData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Anim_Model_Component::Draw()
{
	if (!m_model) return;
	if (m_model->ExistsYet() && m_fence != nullptr) {
		const auto state = glClientWaitSync(m_fence, 0, 0);
		if (((state == GL_ALREADY_SIGNALED) || (state == GL_CONDITION_SATISFIED))
			&& (state != GL_WAIT_FAILED)) {
			shared_lock<shared_mutex> guard(m_model->m_mutex);
			glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
			glBindVertexArray(m_vao_id);
			glDrawArrays(GL_TRIANGLES, 0, m_model->mesh_size);
			glBindVertexArray(0);
		}
	}
}

bool Anim_Model_Component::IsVisible(const mat4 & PMatrix, const mat4 &VMatrix)
{
	if (m_model) {
		shared_lock<shared_mutex> guard(m_model->m_mutex);
		Frustum frustum(PMatrix * VMatrix * m_uboData.mMatrix);

		return frustum.AABBInFrustom(m_model->bbox_min, m_model->bbox_max);
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
			if (m_observer)
				m_observer.reset();
			Asset_Loader::load_asset(m_model, payload);
			m_observer = make_shared<Model_Observer>(m_model, m_vao_id, &m_uboData, &m_skin, m_uboID, &m_fence);
			break;
		}	
		case SET_MODEL_TRANSFORM: {
			if (!message.IsOfType<Transform>()) break;
			const auto &payload = message.GetPayload<Transform>();
			m_uboData.mMatrix = payload.modelMatrix;
			Update();
			break;
		}
		case SET_MODEL_SKIN: {
			if (!message.IsOfType<GLuint>()) break;
			const auto &payload = message.GetPayload<GLuint>();
			m_skin = payload;
			if (m_model->ExistsYet()) {
				m_uboData.materialID = m_model->GetSkinID(m_skin);
				Update();
			}
			break;
		}
	}
}

void Model_Observer::Notify_Finalized()
{
	if (m_asset->ExistsYet()) {// in case this gets used more than once by mistake
		m_asset->UpdateVAO(m_vao_id);
		m_uboData->materialID = m_asset->GetSkinID(*m_skin);
		glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_ubo_id);
		glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_id);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Transform_Buffer), m_uboData);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		*m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glFlush();
	}
}
