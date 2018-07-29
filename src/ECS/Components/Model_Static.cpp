#include "ECS\Components\Model_Static.h"
#include "ECS\ECSmessage.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Engine.h"
#include "glm\gtx\component_wise.hpp"
#include <minmax.h>


Model_Static_C::~Model_Static_C()
{
	if (m_model.get()) {
		m_model->removeCallback(this);
		m_engine->getAssetManager().removeNotifyee(this);
	}
	m_engine->getSubSystem<System_Graphics>("Graphics")->m_geometryBuffers.m_geometryStaticSSBO.removeElement(&m_uboIndex);
}

Model_Static_C::Model_Static_C(Engine * engine, const std::string & filename, const unsigned int & skinIndex, const Transform & transform)
	: Geometry_C(engine)
{
	// Default Parameters
	m_vaoLoaded = false;
	m_skin = 0;
	m_bsphereRadius = 0;
	m_bspherePos = glm::vec3(0.0f);

	// Acquire and update buffers
	m_uboBuffer = m_engine->getSubSystem<System_Graphics>("Graphics")->m_geometryBuffers.m_geometryStaticSSBO.addElement(&m_uboIndex);
	
	// Register Commands
	m_commandMap["Set_Model_Directory"] = [&](const ECS_Command & payload) { 
		if (payload.isType<std::string>()) setModelDirectory(payload.toType<std::string>()); 
	};
	m_commandMap["Set_Skin"] = [&](const ECS_Command & payload) {
		if (payload.isType<int>()) setSkin(payload.toType<int>());
	};
	m_commandMap["Set_Transform"] = [&](const ECS_Command & payload) {
		if (payload.isType<Transform>()) setTransform(payload.toType<Transform>());
	};

	// Update with passed parameters
	setModelDirectory(filename);
	setSkin(skinIndex);
	setTransform(transform);
}

Model_Static_C * Model_Static_C::Create(const ArgumentList & argumentList)
{
	return new Model_Static_C(
		argumentList.checkParameter<Engine>(0) ? (Engine*)argumentList.dataPointers[0] : nullptr,
		argumentList.checkParameter<std::string>(1) ? *(std::string*)argumentList.dataPointers[1] : std::string(""),
		argumentList.checkParameter<unsigned int>(2) ? *(unsigned int*)argumentList.dataPointers[2] : 0,
		argumentList.checkParameter<Transform>(3) ? *(Transform*)argumentList.dataPointers[3] : Transform()
	);
}

bool Model_Static_C::isLoaded() const
{
	if (m_model)
		return m_model->existsYet();
	return false;
}

bool Model_Static_C::isVisible(const float & radius, const glm::vec3 & eyePosition) const
{
	if (m_model && m_model->existsYet()) {
		const float distance = glm::distance(m_bspherePos, eyePosition);
		return radius + m_bsphereRadius > distance;
	}
	return false;
}

bool Model_Static_C::containsPoint(const glm::vec3 & point) const
{
	if (m_model) {
		const float distance = glm::distance(m_bspherePos, point);
		return m_bsphereRadius > distance;
	}
	return false;
}

const unsigned int Model_Static_C::getBufferIndex() const
{
	return m_uboIndex;
}

const glm::ivec2 Model_Static_C::getDrawInfo() const
{
	if (m_model && m_model->existsYet()) {
		std::shared_lock<std::shared_mutex> guard(m_model->m_mutex);
		return glm::ivec2(m_model->m_offset, m_model->m_count);
	}
	return glm::ivec2(0);
}

const unsigned int Model_Static_C::getMeshSize() const
{
	if (m_model && m_model->existsYet()) {
		std::shared_lock<std::shared_mutex> guard(m_model->m_mutex);
		return m_model->m_meshSize;
	}
	return 0;
}

void Model_Static_C::updateBSphere()
{
	if (m_model && m_model->existsYet()) {
		std::shared_lock<std::shared_mutex> guard(m_model->m_mutex);
		const glm::vec3 bboxMax_World = (m_model->m_bboxMax * m_transform.m_scale) + m_transform.m_position;
		const glm::vec3 bboxMin_World = (m_model->m_bboxMin * m_transform.m_scale) + m_transform.m_position;
		const glm::vec3 bboxCenter = (bboxMax_World + bboxMin_World) / 2.0f;	
		const glm::vec3 bboxScale = (bboxMax_World - bboxMin_World) / 2.0f;
		glm::mat4 matTrans = glm::translate(glm::mat4(1.0f), bboxCenter);
		glm::mat4 matRot = glm::mat4_cast(m_transform.m_orientation);
		glm::mat4 matScale = glm::scale(glm::mat4(1.0f), bboxScale);
		glm::mat4 matFinal = (matTrans * matRot * matScale);
		(&reinterpret_cast<Geometry_Static_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->bBoxMatrix = matFinal;
		m_bsphereRadius = glm::compMax(m_model->m_radius * m_transform.m_scale);
		m_bspherePos = m_model->m_bboxCenter + m_transform.m_position;
	}
}

void Model_Static_C::setModelDirectory(const std::string & directory)
{
	if (directory == "") return;
	// Remove callback from old model before loading
	if (m_model.get())
		m_model->removeCallback(this);
	// Load new model
	m_vaoLoaded = false;
	m_engine->createAsset(m_model, directory, true);
	// Attach new callback
	m_model->addCallback(this, [&]() {
		(&reinterpret_cast<Geometry_Static_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->materialID = m_model->getSkinID(m_skin);
		updateBSphere();
		m_vaoLoaded = true;
	});
}

void Model_Static_C::setSkin(const unsigned int & index)
{
	m_skin = index;
	if (m_model && m_model->existsYet())
		(&reinterpret_cast<Geometry_Static_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->materialID = m_model->getSkinID(m_skin);
}

void Model_Static_C::setTransform(const Transform & transform)
{
	(&reinterpret_cast<Geometry_Static_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->mMatrix = transform.m_modelMatrix;
	if (m_model && m_model->existsYet())
		updateBSphere();
	else
		(&reinterpret_cast<Geometry_Static_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->bBoxMatrix = transform.m_modelMatrix;
	m_transform = transform;
}
