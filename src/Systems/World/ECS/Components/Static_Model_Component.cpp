#include "Systems\World\ECS\Components\Static_Model_Component.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Utilities\EnginePackage.h"
#include "glm\gtx\component_wise.hpp"
#include <minmax.h>


Static_Model_Component::~Static_Model_Component()
{
	if (m_model.get()) m_model->removeCallback(this);
	m_enginePackage->getSubSystem<System_Graphics>("Graphics")->m_geometryBuffers.m_geometryStaticSSBO.removeElement(&m_uboIndex);
}

Static_Model_Component::Static_Model_Component(EnginePackage *enginePackage)
{
	m_enginePackage = enginePackage;
	m_vaoLoaded = false;
	m_skin = 0;
	m_bsphereRadius = 0;
	m_bspherePos = vec3(0.0f);

	m_uboBuffer = m_enginePackage->getSubSystem<System_Graphics>("Graphics")->m_geometryBuffers.m_geometryStaticSSBO.addElement(&m_uboIndex);
	m_commandMap["Set_Model_Directory"] = [&](const ECS_Command & payload) { 
		if (payload.isType<string>()) setModelDirectory(payload.toType<string>()); 
	};
	m_commandMap["Set_Skin"] = [&](const ECS_Command & payload) {
		if (payload.isType<int>()) setSkin(payload.toType<int>());
	};
	m_commandMap["Set_Transform"] = [&](const ECS_Command & payload) {
		if (payload.isType<Transform>()) setTransform(payload.toType<Transform>());
	};
}

bool Static_Model_Component::isLoaded() const
{
	if (m_model)
		return m_model->existsYet();
	return false;
}

bool Static_Model_Component::isVisible(const float & radius, const vec3 & eyePosition) const
{
	if (m_model && m_model->existsYet()) {
		const float distance = glm::distance(m_bspherePos, eyePosition);
		return radius + m_bsphereRadius > distance;
	}
	return false;
}

bool Static_Model_Component::containsPoint(const vec3 & point) const
{
	if (m_model) {
		const float distance = glm::distance(m_bspherePos, point);
		return m_bsphereRadius > distance;
	}
	return false;
}

const unsigned int Static_Model_Component::getBufferIndex() const
{
	return m_uboIndex;
}

const ivec2 Static_Model_Component::getDrawInfo() const
{
	if (m_model && m_model->existsYet()) {
		shared_lock<shared_mutex> guard(m_model->m_mutex);
		return ivec2(m_model->m_offset, m_model->m_count);
	}
	return ivec2(0);
}

const unsigned int Static_Model_Component::getMeshSize() const
{
	if (m_model && m_model->existsYet()) {
		shared_lock<shared_mutex> guard(m_model->m_mutex);
		return m_model->m_meshSize;
	}
	return 0;
}

void Static_Model_Component::updateBSphere()
{
	if (m_model && m_model->existsYet()) {
		shared_lock<shared_mutex> guard(m_model->m_mutex);
		const vec3 bboxMax_World = (m_model->m_bboxMax * m_transform.m_scale) + m_transform.m_position;
		const vec3 bboxMin_World = (m_model->m_bboxMin * m_transform.m_scale) + m_transform.m_position;
		const vec3 bboxCenter = (bboxMax_World + bboxMin_World) / 2.0f;	
		const vec3 bboxScale = (bboxMax_World - bboxMin_World) / 2.0f;
		mat4 matTrans = glm::translate(mat4(1.0f), bboxCenter);
		mat4 matRot = glm::mat4_cast(m_transform.m_orientation);
		mat4 matScale = glm::scale(mat4(1.0f), bboxScale);
		mat4 matFinal = (matTrans * matRot * matScale);
		(&reinterpret_cast<Geometry_Static_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->bBoxMatrix = matFinal;
		m_bsphereRadius = glm::compMax(m_model->m_radius * m_transform.m_scale);
		m_bspherePos = m_model->m_bboxCenter + m_transform.m_position;
	}
}

void Static_Model_Component::setModelDirectory(const string & directory)
{
	// Remove callback from old model before loading
	if (m_model.get())
		m_model->removeCallback(this);
	// Load new model
	m_vaoLoaded = false;
	Asset_Model::Create(m_model, directory);
	// Attach new callback
	m_model->addCallback(this, [&]() {
		(&reinterpret_cast<Geometry_Static_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->materialID = m_model->getSkinID(m_skin);
		updateBSphere();
		m_vaoLoaded = true;
	});
}

void Static_Model_Component::setSkin(const unsigned int & index)
{
	m_skin = index;
	if (m_model && m_model->existsYet())
		(&reinterpret_cast<Geometry_Static_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->materialID = m_model->getSkinID(m_skin);
}

void Static_Model_Component::setTransform(const Transform & transform)
{
	(&reinterpret_cast<Geometry_Static_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->mMatrix = transform.m_modelMatrix;
	if (m_model && m_model->existsYet())
		updateBSphere();
	else
		(&reinterpret_cast<Geometry_Static_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->bBoxMatrix = transform.m_modelMatrix;
	m_transform = transform;
}
