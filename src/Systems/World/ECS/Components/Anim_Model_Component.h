#pragma once
#ifndef ANIM_MODEL_COMPONENT
#define ANIM_MODEL_COMPONENT
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZ

#include "Systems\World\ECS\Components\Geometry_Component.h"
#include "Assets\Asset_Model.h"
#include "glm\glm.hpp"

using namespace glm;
class Anim_Model_Creator;
class Model_Observer;


/**
 * A renderable model component that supports animation.
 **/
class DT_ENGINE_API Anim_Model_Component : protected Geometry_Component
{
public:
	// Interface implementations
	virtual void receiveMessage(const ECSmessage &message);
	virtual void draw();
	virtual bool isVisible(const mat4 & PMatrix, const mat4 &VMatrix);


	// Public Methods
	/** Sends current data to the GPU. */
	void update();
	/** Ticks ahead the animation state by the amount of deltaTime. */
	void animate(const double &deltaTime);


protected:
	/** Nested Buffer class.
	 * @brief	used for sending data to the gpu. */
	struct Transform_Buffer {
		int useBones;  // no padding here;
		GLuint materialID; vec2 padding1; // for some reason padding here
		mat4 mMatrix;
		mat4 transforms[NUM_MAX_BONES];
		Transform_Buffer() {
			useBones = 0;
			materialID = 0;
			mMatrix = mat4(1.0f);
		}
	};


	// (de)Constructors
	/** Destroys an animated model component. */
	~Anim_Model_Component();
	/** Constructors an animated model component. */
	Anim_Model_Component(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage);


	// Protected Attributes
	int m_animation;
	bool m_playAnim;
	float m_animTime, m_animStart;
	GLuint m_skin;
	GLuint m_uboID, m_vao_id;
	Transform_Buffer m_uboData;
	Shared_Asset_Model m_model;
	vector<BoneInfo> m_transforms;
	unique_ptr<Model_Observer> m_observer;
	GLsync m_fence; 
	friend class Anim_Model_Creator;
	friend class Model_Observer;
};

class DT_ENGINE_API Anim_Model_Creator : public ComponentCreator
{
public:
	Anim_Model_Creator(ECSmessenger *ecsMessenger) : ComponentCreator(ecsMessenger) {}
	virtual Component* Create(const ECShandle &id, const ECShandle &pid, EnginePackage *enginePackage) {
		return new Anim_Model_Component(id, pid, enginePackage);
	}
};

struct DT_ENGINE_API Model_Observer : Asset_Observer
{
	Model_Observer(Shared_Asset_Model & asset, vector<BoneInfo> *transforms, const GLuint &vao, Anim_Model_Component::Transform_Buffer * uboData, GLuint *skin, const GLuint &uboID, GLsync *fence) :
		Asset_Observer(asset.get()), m_vao_id(vao), m_transforms(transforms), m_uboData(uboData), m_skin(skin), m_ubo_id(uboID), m_fence(fence)  {};
	virtual void Notify_Finalized();

	GLuint m_vao_id, m_ubo_id;
	GLsync *m_fence;
	GLuint *m_skin;
	Anim_Model_Component::Transform_Buffer *m_uboData;
	vector<BoneInfo> *m_transforms;
};

#endif // ANIM_MODEL_COMPONENT