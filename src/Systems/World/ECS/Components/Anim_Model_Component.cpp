#include "Systems\World\ECS\Components\Anim_Model_Component.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Utilities\Frustum.h"
#include "Utilities\Transform.h"
#include <minmax.h>


inline void ReadNodeHeirarchy(vector<BoneInfo> &transforms, const float &animation_time, const int &animation_ID, const aiNode* parentNode, const Shared_Asset_Model &model, const mat4& ParentTransform);

Anim_Model_Component::~Anim_Model_Component()
{
	glDeleteBuffers(1, &m_uboID);
	glDeleteVertexArrays(1, &m_vao_id);
	if (m_model.get()) m_model->removeCallback(Asset::FINALIZED, this);
}

Anim_Model_Component::Anim_Model_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage) : Geometry_Component(id, pid)
{
	m_animation = 0;
	m_animTime = 0;
	m_animStart = 0;
	m_playAnim = false;
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
	
	m_vao_id = Asset_Model::Generate_VAO();	
}

void Anim_Model_Component::receiveMessage(const ECSmessage &message)
{
	if (Component::compareMSGSender(message)) return;
	switch (message.GetCommandID()) {
	case SET_MODEL_DIR: {
		if (!message.IsOfType<string>()) break;
		const auto &payload = message.GetPayload<string>();
		// Remove callback from old model before loading
		if (m_model.get()) 
			m_model->removeCallback(Asset::FINALIZED, this);
		// Load new model
		Asset_Loader::load_asset(m_model, payload);
		// Attach new callback
		m_model->addCallback(Asset::FINALIZED, this, [&]() {
			if (m_model->existsYet()) {
				m_model->updateVAO(m_vao_id);
				m_uboData.materialID = m_model->getSkinID(m_skin);
				m_transforms = m_model->m_animationInfo.meshTransforms;
				glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
				glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Anim_Model_Component::Transform_Buffer), &m_uboData);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);

				m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
				glFlush();
			}
		});
		break;
	}
	case SET_MODEL_TRANSFORM: {
		if (!message.IsOfType<Transform>()) break;
		const auto &payload = message.GetPayload<Transform>();
		m_uboData.mMatrix = payload.m_modelMatrix;
		update();
		break;
	}
	case SET_MODEL_SKIN: {
		if (!message.IsOfType<GLuint>()) break;
		const auto &payload = message.GetPayload<GLuint>();
		m_skin = payload;
		if (m_model->existsYet()) {
			m_uboData.materialID = m_model->getSkinID(m_skin);
			update();
		}
		break;
	}
	case SET_MODEL_ANIMATION: {
		if (message.IsOfType<int>()) {
			m_animation = message.GetPayload<int>();
			m_playAnim = true;
		}
		else if (message.IsOfType<bool>()) {
			m_playAnim = message.GetPayload<bool>();
		}
		break;
	}
	}
}

void Anim_Model_Component::draw()
{
	if (!m_model) return;
	if (m_model->existsYet() && m_fence != nullptr) {
		const auto state = glClientWaitSync(m_fence, 0, 0);
		if (((state == GL_ALREADY_SIGNALED) || (state == GL_CONDITION_SATISFIED))
			&& (state != GL_WAIT_FAILED)) {
			shared_lock<shared_mutex> guard(m_model->m_mutex);
			glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
			glBindVertexArray(m_vao_id);
			glDrawArrays(GL_TRIANGLES, 0, m_model->m_meshSize);
			glBindVertexArray(0);
		}
	}
}

bool Anim_Model_Component::isVisible(const mat4 & PMatrix, const mat4 &VMatrix)
{
	if (m_model) {
		shared_lock<shared_mutex> guard(m_model->m_mutex);
		Frustum frustum(PMatrix * VMatrix * m_uboData.mMatrix);

		return frustum.AABBInFrustom(m_model->m_bboxMin, m_model->m_bboxMax);
	}
	return false;
}

void Anim_Model_Component::update()
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Transform_Buffer), &m_uboData);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Anim_Model_Component::animate(const double & deltaTime)
{
	if (!m_model->existsYet()) return;

	shared_lock<shared_mutex> guard(m_model->m_mutex);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);

	if (m_animation == -1 || m_transforms.size() == 0 || m_animation >= m_model->m_animationInfo.Animations.size()) {
		m_uboData.useBones = 0;
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Transform_Buffer, useBones), sizeof(int), &m_uboData.useBones);
	}
	else {
		m_uboData.useBones = 1;
		if (m_playAnim)
			m_animTime += deltaTime;
		const float TicksPerSecond = m_model->m_animationInfo.Animations[m_animation]->mTicksPerSecond != 0
			? m_model->m_animationInfo.Animations[m_animation]->mTicksPerSecond
			: 25.0f;
		const float TimeInTicks = m_animTime * TicksPerSecond;
		const float AnimationTime = fmod(TimeInTicks, m_model->m_animationInfo.Animations[m_animation]->mDuration);
		m_animStart = m_animStart == -1 ? TimeInTicks : m_animStart;

		ReadNodeHeirarchy(m_transforms, AnimationTime, m_animation, m_model->m_animationInfo.RootNode, m_model, mat4(1.0f));

		// Lock guard goes here
		const unsigned int total = min(m_transforms.size(), NUM_MAX_BONES);
		for (int i = 0; i < total; i++)
			m_uboData.transforms[i] = m_transforms[i].FinalTransformation;
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Transform_Buffer, useBones), sizeof(int), &m_uboData.useBones);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Transform_Buffer, transforms), sizeof(mat4) * total, &m_uboData.transforms);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

template <typename FROM, typename TO> inline TO convertType(const FROM &t) { return *((TO*)&t); }
template <> inline quat convertType(const aiQuaternion &t) { return quat(t.w, t.x, t.y, t.z); }
template <> inline mat4 convertType(const aiMatrix4x4 &t) { return mat4(t.a1, t.b1, t.c1, t.d1,	t.a2, t.b2, t.c2, t.d2,	t.a3, t.b3, t.c3, t.d3,	t.a4, t.b4, t.c4, t.d4); }
template <typename T> inline T valueMix(const T &t1, const T &t2, const float &f) { return glm::mix(t1, t2, f); }
template <> inline quat valueMix(const quat &t1, const quat &t2, const float &f) { return glm::slerp(t1, t2, f); }

inline const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string &NodeName)
{
	for (unsigned int i = 0; i < pAnimation->mNumChannels; i++) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];
		if (string(pNodeAnim->mNodeName.data) == NodeName) 
			return pNodeAnim;		
	}
	return nullptr;
}

template <typename T>
inline int FindKey(const float &AnimationTime, const unsigned int &count, const T *keys) 
{
	for (unsigned int i = 0; i < count; i++)
		if (AnimationTime < (float)(keys[i + 1]).mTime)
			return i;
	return 0;
}

template <typename KeyType, typename FromType, typename DesiredType>
inline DesiredType InterpolateKeys(const float &AnimationTime, const unsigned int &keyCount, KeyType *keys) {
	assert(keyCount > 0);
	DesiredType &Result = convertType<FromType, DesiredType>(keys[0].mValue);
	if (keyCount > 1) { // Ensure we have 2 values to interpolate between
		unsigned int Index = FindKey<KeyType>(AnimationTime, keyCount - 1, keys);
		unsigned int NextIndex = (Index + 1) > keyCount ? 0 : (Index + 1);
		const KeyType &Key = keys[Index];
		const KeyType &NextKey = keys[NextIndex];
		const float DeltaTime = (float)(NextKey.mTime - Key.mTime);
		const float Factor = clamp((AnimationTime - (float)Key.mTime) / DeltaTime, 0.0f, 1.0f);
		const DesiredType Start = convertType<FromType, DesiredType>(Key.mValue);
		const DesiredType End = convertType<FromType, DesiredType>(NextKey.mValue);
		return valueMix(Start, End, Factor);
	}
	return Result;
}

inline void ReadNodeHeirarchy(vector<BoneInfo> &transforms, const float &animation_time, const int &animation_ID, const aiNode* parentNode, const Shared_Asset_Model &model, const mat4& ParentTransform)
{
	const string NodeName(parentNode->mName.data);
	const aiAnimation* pAnimation = model->m_animationInfo.Animations[animation_ID];
	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);
	mat4 NodeTransformation = convertType<aiMatrix4x4, mat4>(parentNode->mTransformation);

	// Interpolate scaling, rotation, and translation.
	// Generate their matrices and apply their transformations.
	if (pNodeAnim) {
		const vec3 Scaling = InterpolateKeys<aiVectorKey, aiVector3D, vec3>
							(animation_time, pNodeAnim->mNumScalingKeys, pNodeAnim->mScalingKeys);
		const quat Rotation = InterpolateKeys<aiQuatKey, aiQuaternion, quat>
							(animation_time, pNodeAnim->mNumRotationKeys, pNodeAnim->mRotationKeys);
		const vec3 Translation = InterpolateKeys<aiVectorKey, aiVector3D, vec3>
							(animation_time, pNodeAnim->mNumPositionKeys, pNodeAnim->mPositionKeys);
		
		NodeTransformation = glm::translate(mat4(1.0f), Translation) * glm::mat4_cast(Rotation) * glm::scale(mat4(1.0f), Scaling);
	}

	const mat4 GlobalTransformation = ParentTransform * NodeTransformation;
	const mat4 GlobalInverseTransform = glm::inverse(convertType<aiMatrix4x4, mat4>(model->m_animationInfo.RootNode->mTransformation));
	const map<string, int> &BoneMap = model->m_animationInfo.boneMap;
	if (BoneMap.find(NodeName) != BoneMap.end()) {
		int BoneIndex = BoneMap.at(NodeName);
		transforms.at(BoneIndex).FinalTransformation = GlobalInverseTransform * GlobalTransformation * transforms.at(BoneIndex).BoneOffset;
	}

	for (unsigned int i = 0; i < parentNode->mNumChildren; i++)
		ReadNodeHeirarchy(transforms, animation_time, animation_ID, parentNode->mChildren[i], model, GlobalTransformation);
}