#include "ECS\Components\Model_Animated.h"
#include "ECS\ECSmessage.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Engine.h"
#include "glm\gtx\component_wise.hpp"
#include <minmax.h>


inline void ReadNodeHeirarchy(std::vector<BoneTransform> &transforms, const float &animation_time, const int &animation_ID, const Node* parentNode, const Shared_Asset_Model &model, const glm::mat4& ParentTransform);

Model_Animated_C::~Model_Animated_C()
{
	if (m_model.get()) m_model->removeCallback(this);
	m_engine->getSubSystem<System_Graphics>("Graphics")->m_geometryBuffers.m_geometryDynamicSSBO.removeElement(&m_uboIndex);
}

Model_Animated_C::Model_Animated_C(Engine *engine)
{
	m_engine = engine;
	m_vaoLoaded = false;
	m_animation = -1;
	m_animTime = 0;
	m_animStart = 0;
	m_playAnim = false;
	m_skin = 0;
	m_bsphereRadius = 0;
	m_bspherePos = glm::vec3(0.0f);

	m_uboBuffer = m_engine->getSubSystem<System_Graphics>("Graphics")->m_geometryBuffers.m_geometryDynamicSSBO.addElement(&m_uboIndex);
	m_commandMap["Set_Model_Directory"] = [&](const ECS_Command & payload) {
		if (payload.isType<std::string>()) setModelDirectory(payload.toType<std::string>());
	};
	m_commandMap["Set_Skin"] = [&](const ECS_Command & payload) {
		if (payload.isType<int>()) setSkin(payload.toType<int>());
	};
	m_commandMap["Set_Animation"] = [&](const ECS_Command & payload) {
		if (payload.isType<int>()) setAnimation(payload.toType<int>());
	};
	m_commandMap["Set_Transform"] = [&](const ECS_Command & payload) {
		if (payload.isType<Transform>()) setTransform(payload.toType<Transform>());
	};
}

bool Model_Animated_C::isLoaded() const
{
	if (m_model)
		return m_model->existsYet();
	return false;
}

bool Model_Animated_C::isVisible(const float & radius, const glm::vec3 & eyePosition) const
{
	if (m_model && m_model->existsYet()) {
		const float distance = glm::distance(m_bspherePos, eyePosition);
		return radius + m_bsphereRadius > distance;
	}
	return false;
}

bool Model_Animated_C::containsPoint(const glm::vec3 & point) const
{
	if (m_model) {
		const float distance = glm::distance(m_bspherePos, point);
		return m_bsphereRadius > distance;
	}
	return false;
}

const unsigned int Model_Animated_C::getBufferIndex() const
{
	return m_uboIndex;
}

const glm::ivec2 Model_Animated_C::getDrawInfo() const
{
	if (m_model && m_model->existsYet()) {
		std::shared_lock<std::shared_mutex> guard(m_model->m_mutex);
		return glm::ivec2(m_model->m_offset, m_model->m_count);
	}
	return glm::ivec2(0);
}

const unsigned int Model_Animated_C::getMeshSize() const
{
	if (m_model && m_model->existsYet()) {
		std::shared_lock<std::shared_mutex> guard(m_model->m_mutex);
		return m_model->m_meshSize;
	}
	return 0;
}

void Model_Animated_C::updateBSphere()
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
		(&reinterpret_cast<Geometry_Dynamic_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->bBoxMatrix = matFinal;
		m_bsphereRadius = glm::compMax(m_model->m_radius * m_transform.m_scale);
		m_bspherePos = m_model->m_bboxCenter + m_transform.m_position;
	}
}

void Model_Animated_C::setModelDirectory(const std::string & directory)
{
	// Remove callback from old model before loading
	if (m_model.get())
		m_model->removeCallback(this);
	// Load new model
	m_vaoLoaded = false;
	m_engine->createAsset(m_model, directory, true);
	// Attach new callback
	m_model->addCallback(this, [&]() {
		(&reinterpret_cast<Geometry_Dynamic_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->materialID = m_model->getSkinID(m_skin);
		m_transforms = m_model->m_boneTransforms;
		updateBSphere();
		m_vaoLoaded = true;
	});
}

void Model_Animated_C::setSkin(const unsigned int & index)
{
	m_skin = index;
	if (m_model && m_model->existsYet())
		(&reinterpret_cast<Geometry_Dynamic_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->materialID = m_model->getSkinID(m_skin);
}

void Model_Animated_C::setAnimation(const unsigned int & index)
{
	m_animation = index;
	m_playAnim = true;
	//m_playAnim = message.GetPayload<bool>();
}

void Model_Animated_C::setTransform(const Transform & transform)
{
	(&reinterpret_cast<Geometry_Dynamic_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->mMatrix = transform.m_modelMatrix;
	if (m_model && m_model->existsYet())
		updateBSphere();
	else
		(&reinterpret_cast<Geometry_Dynamic_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->bBoxMatrix = transform.m_modelMatrix;
	m_transform = transform;
}


void Model_Animated_C::animate(const double & deltaTime)
{
	if (!m_model || !m_model->existsYet()) return;
	
	Geometry_Dynamic_Struct *const uboData = &reinterpret_cast<Geometry_Dynamic_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	std::shared_lock<std::shared_mutex> guard(m_model->m_mutex);

	if (m_animation == -1 || m_transforms.size() == 0 || m_animation >= m_model->m_animations.size())
		uboData->useBones = 0;	
	else {
		uboData->useBones = 1;
		if (m_playAnim)
			m_animTime += deltaTime;
		const float TicksPerSecond = m_model->m_animations[m_animation].ticksPerSecond != 0
			? m_model->m_animations[m_animation].ticksPerSecond
			: 25.0f;
		const float TimeInTicks = m_animTime * TicksPerSecond;
		const float AnimationTime = fmod(TimeInTicks, m_model->m_animations[m_animation].duration);
		m_animStart = m_animStart == -1 ? TimeInTicks : m_animStart;

		ReadNodeHeirarchy(m_transforms, AnimationTime, m_animation, m_model->m_rootNode, m_model, glm::mat4(1.0f));

		for (unsigned int i = 0, total = min(m_transforms.size(), NUM_MAX_BONES); i < total; i++)
			uboData->transforms[i] = m_transforms[i].final;		
	}
}

template <typename FROM, typename TO> inline TO convertType(const FROM &t) { return *((TO*)&t); }
template <> inline glm::quat convertType(const aiQuaternion &t) { return glm::quat(t.w, t.x, t.y, t.z); }
template <> inline glm::mat4 convertType(const aiMatrix4x4 &t) { return glm::mat4(t.a1, t.b1, t.c1, t.d1,	t.a2, t.b2, t.c2, t.d2,	t.a3, t.b3, t.c3, t.d3,	t.a4, t.b4, t.c4, t.d4); }
template <typename T> inline T valueMix(const T &t1, const T &t2, const float &f) { return glm::mix(t1, t2, f); }
template <> inline glm::quat valueMix(const glm::quat &t1, const glm::quat &t2, const float &f) { return glm::slerp(t1, t2, f); }

inline const Node_Animation * FindNodeAnim(const Animation & pAnimation, const std::string &NodeName)
{
	for (unsigned int i = 0; i < pAnimation.numChannels; i++) {
		const Node_Animation * pNodeAnim = pAnimation.channels[i];
		if (pNodeAnim->nodeName == NodeName)
			return pNodeAnim;		
	}
	return nullptr;
}

template <typename KeyType>
inline int FindKey(const float &AnimationTime, const unsigned int &count, const std::vector<KeyType> & keys)
{
	for (unsigned int i = 0; i < count; i++)
		if (AnimationTime < (float)(keys[i + 1]).time)
			return i;
	return 0;
}

template <typename ValueType>
inline ValueType InterpolateKeys(const float &AnimationTime, const std::vector<Animation_Time_Key<ValueType>> & keys) {
	const unsigned int & keyCount = keys.size();
	assert(keyCount > 0);
	const ValueType &Result = keys[0].value;
	if (keyCount > 1) { // Ensure we have 2 values to interpolate between
		unsigned int Index = FindKey<Animation_Time_Key<ValueType>>(AnimationTime, keyCount - 1, keys);
		unsigned int NextIndex = (Index + 1) > keyCount ? 0 : (Index + 1);
		const Animation_Time_Key<ValueType> &Key = keys[Index];
		const Animation_Time_Key<ValueType> &NextKey = keys[NextIndex];
		const float DeltaTime = (float)(NextKey.time - Key.time);
		const float Factor = glm::clamp((AnimationTime - (float)Key.time) / DeltaTime, 0.0f, 1.0f);
		return valueMix(Key.value, NextKey.value, Factor);
	}
	return Result;
}

inline void ReadNodeHeirarchy(std::vector<BoneTransform> &transforms, const float &animation_time, const int &animation_ID, const Node* parentNode, const Shared_Asset_Model &model, const glm::mat4& ParentTransform)
{
	const std::string & NodeName = parentNode->name;
	const Animation &pAnimation = model->m_animations[animation_ID];
	const Node_Animation *pNodeAnim = FindNodeAnim(pAnimation, NodeName);
	glm::mat4 NodeTransformation = parentNode->transformation;

	// Interpolate scaling, rotation, and translation.
	// Generate their matrices and apply their transformations.
	if (pNodeAnim) {
		const glm::vec3 Scaling = InterpolateKeys<glm::vec3>(animation_time, pNodeAnim->scalingKeys);
		const glm::quat Rotation = InterpolateKeys<glm::quat>(animation_time, pNodeAnim->rotationKeys);
		const glm::vec3 Translation = InterpolateKeys<glm::vec3>(animation_time, pNodeAnim->positionKeys);
		
		NodeTransformation = glm::translate(glm::mat4(1.0f), Translation) * glm::mat4_cast(Rotation) * glm::scale(glm::mat4(1.0f), Scaling);
	}

	const glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;
	const glm::mat4 GlobalInverseTransform = glm::inverse(model->m_rootNode->transformation);
	const std::map<std::string, int> &BoneMap = model->m_boneMap;
	if (BoneMap.find(NodeName) != BoneMap.end()) {
		int BoneIndex = BoneMap.at(NodeName);
		transforms.at(BoneIndex).final = GlobalInverseTransform * GlobalTransformation * transforms.at(BoneIndex).offset;
	}

	for (unsigned int i = 0; i < parentNode->children.size(); ++i)
		ReadNodeHeirarchy(transforms, animation_time, animation_ID, parentNode->children[i], model, GlobalTransformation);
}