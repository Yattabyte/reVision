#include "Entities\Components\Anim_Model_Component.h"
#include "Systems\World\ECSmessage.h"
#include "Systems\World\ECSmessages.h"
#include "Utilities\Frustum.h"
#include "Utilities\Transform.h"
#include <minmax.h>

Anim_Model_Component::~Anim_Model_Component()
{
	glDeleteBuffers(1, &m_uboID);
	glDeleteVertexArrays(1, &m_vao_id);
}

Anim_Model_Component::Anim_Model_Component(const ECShandle &id, const ECShandle &pid, Engine_Package *enginePackage) : Geometry_Component(id, pid)
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
			m_observer = make_unique<Model_Observer>(m_model, &m_transforms, m_vao_id, &m_uboData, &m_skin, m_uboID, &m_fence);
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
		case PLAY_ANIMATION: {
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

void Model_Observer::Notify_Finalized()
{
	if (m_asset->ExistsYet()) {// in case this gets used more than once by mistake
		m_asset->UpdateVAO(m_vao_id);
		m_uboData->materialID = m_asset->GetSkinID(*m_skin);
		*m_transforms = m_asset->animationInfo.meshTransforms;
		glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_ubo_id);
		glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_id);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Transform_Buffer), m_uboData);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		*m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		glFlush();
	}
}

inline mat4 aiMatrixtoMat4x4(const aiMatrix4x4 &d)
{
	return mat4(d.a1, d.b1, d.c1, d.d1,
		d.a2, d.b2, d.c2, d.d2,
		d.a3, d.b3, d.c3, d.d3,
		d.a4, d.b4, d.c4, d.d4);
}

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

inline mat4 CalcInterpolatedScaling(const float &AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);
	auto Result = pNodeAnim->mScalingKeys[0].mValue;	
	if (pNodeAnim->mNumScalingKeys > 1) { // Ensure we have 2 values to interpolate between
		unsigned int ScalingIndex = FindKey<aiVectorKey>(AnimationTime, pNodeAnim->mNumScalingKeys - 1, pNodeAnim->mScalingKeys);
		unsigned int NextScalingIndex = (ScalingIndex + 1) > pNodeAnim->mNumScalingKeys ? 0 : (ScalingIndex + 1);
		const auto &Key = pNodeAnim->mScalingKeys[ScalingIndex];
		const auto &NextKey = pNodeAnim->mScalingKeys[NextScalingIndex];
		const float DeltaTime = (float)(NextKey.mTime - Key.mTime);
		const float Factor = clamp((AnimationTime - (float)Key.mTime) / DeltaTime, 0.0f, 1.0f);
		const aiVector3D& Start = Key.mValue;
		const aiVector3D& End = NextKey.mValue;
		Result = Start + Factor * (End - Start);
	}
	return glm::scale(mat4(1.0f), vec3(Result.x, Result.y, Result.z));
}

inline mat4 CalcInterpolatedRotation(const float &AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);
	auto Result = pNodeAnim->mRotationKeys[0].mValue;
	if (pNodeAnim->mNumRotationKeys > 1) { // Ensure we have 2 values to interpolate between
		unsigned int RotationIndex = FindKey<aiQuatKey>(AnimationTime, pNodeAnim->mNumRotationKeys - 1, pNodeAnim->mRotationKeys);
		unsigned int NextRotationIndex = (RotationIndex + 1) > pNodeAnim->mNumRotationKeys ? 0 : (RotationIndex + 1);
		const auto &Key = pNodeAnim->mRotationKeys[RotationIndex];
		const auto &NextKey = pNodeAnim->mRotationKeys[NextRotationIndex];
		const float DeltaTime = (float)(NextKey.mTime - Key.mTime);
		const float Factor = clamp((AnimationTime - (float)Key.mTime) / DeltaTime, 0.0f, 1.0f);
		const aiQuaternion& StartRotationQ = Key.mValue;
		const aiQuaternion& EndRotationQ = NextKey.mValue;
		aiQuaternion::Interpolate(Result, StartRotationQ, EndRotationQ, Factor);
		Result.Normalize();
	}
	return aiMatrixtoMat4x4(aiMatrix4x4(Result.GetMatrix()));
}

inline mat4 CalcInterpolatedPosition(const float &AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumPositionKeys > 0);	
	auto Result = pNodeAnim->mPositionKeys[0].mValue;
	if (pNodeAnim->mNumPositionKeys > 1) { // Ensure we have 2 values to interpolate between
		unsigned int PositionIndex = FindKey<aiVectorKey>(AnimationTime, pNodeAnim->mNumPositionKeys - 1, pNodeAnim->mPositionKeys);
		unsigned int NextPositionIndex = (PositionIndex + 1) > pNodeAnim->mNumPositionKeys ? 0 : (PositionIndex + 1);
		const auto &Key = pNodeAnim->mPositionKeys[PositionIndex];
		const auto &NextKey = pNodeAnim->mPositionKeys[NextPositionIndex];
		const float DeltaTime = (float)(NextKey.mTime - Key.mTime);
		const float Factor = clamp((AnimationTime - (float)Key.mTime) / DeltaTime, 0.0f, 1.0f);
		const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
		const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
		Result = Start + Factor * (End - Start);
	}	
	return glm::translate(mat4(1.0f), vec3(Result.x, Result.y, Result.z));
}

inline void ReadNodeHeirarchy(vector<BoneInfo> &transforms, const float &animation_time, const int &animation_ID, const aiNode* parentNode, const Shared_Asset_Model &model, const mat4& ParentTransform)
{
	const string NodeName(parentNode->mName.data);
	const aiAnimation* pAnimation = model->animationInfo.Animations[animation_ID];
	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);
	mat4 NodeTransformation = aiMatrixtoMat4x4(parentNode->mTransformation);

	// Interpolate scaling, rotation, and translation.
	// Generate their matrices and apply their transformations.
	if (pNodeAnim) {
		const mat4 ScalingM = CalcInterpolatedScaling(animation_time, pNodeAnim);
		const mat4 RotationM = CalcInterpolatedRotation(animation_time, pNodeAnim);
		const mat4 TranslationM = CalcInterpolatedPosition(animation_time, pNodeAnim);
		NodeTransformation = TranslationM * RotationM * ScalingM;
	}

	const mat4 GlobalTransformation = ParentTransform * NodeTransformation;
	const mat4 GlobalInverseTransform = glm::inverse(aiMatrixtoMat4x4(model->animationInfo.RootNode->mTransformation));
	const map<string, int> &BoneMap = model->animationInfo.boneMap;
	if (BoneMap.find(NodeName) != BoneMap.end()) {
		int BoneIndex = BoneMap.at(NodeName);
		transforms.at(BoneIndex).FinalTransformation = GlobalInverseTransform * GlobalTransformation * transforms.at(BoneIndex).BoneOffset;
	}

	for (unsigned int i = 0; i < parentNode->mNumChildren; i++)
		ReadNodeHeirarchy(transforms, animation_time, animation_ID, parentNode->mChildren[i], model, GlobalTransformation);
}

void Anim_Model_Component::animate(const double & deltaTime)
{
	if (!m_model->ExistsYet()) return;

	shared_lock<shared_mutex> guard(m_model->m_mutex);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);

	if (m_animation == -1 || m_transforms.size() == 0 || m_animation >= m_model->animationInfo.Animations.size()) {
		m_uboData.useBones = 0;
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Transform_Buffer, useBones), sizeof(int), &m_uboData.useBones);
	}
	else {
		m_uboData.useBones = 1;
		if (m_playAnim)
			m_animTime += deltaTime;
		const float TicksPerSecond = m_model->animationInfo.Animations[m_animation]->mTicksPerSecond != 0
			? m_model->animationInfo.Animations[m_animation]->mTicksPerSecond
			: 25.0f;
		const float TimeInTicks = m_animTime * TicksPerSecond;
		const float AnimationTime = fmod(TimeInTicks, m_model->animationInfo.Animations[m_animation]->mDuration);
		m_animStart = m_animStart == -1 ? TimeInTicks : m_animStart;

		ReadNodeHeirarchy(m_transforms, AnimationTime, m_animation, m_model->animationInfo.RootNode, m_model, mat4(1.0f));

		// Lock guard goes here
		const unsigned int total = min(m_transforms.size(), NUM_MAX_BONES);
		for (int i = 0; i < total; i++)
			m_uboData.transforms[i] = m_transforms[i].FinalTransformation;
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Transform_Buffer, useBones), sizeof(int), &m_uboData.useBones);
		glBufferSubData(GL_UNIFORM_BUFFER, offsetof(Transform_Buffer, transforms), sizeof(mat4) * total, &m_uboData.transforms);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}