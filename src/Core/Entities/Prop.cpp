#include "Prop.h"
#include "Entities\Components\Anim_Model_Component.h"
#include "Managers\Component_Manager.h"

Prop::~Prop()
{
}

Prop::Prop()
{
	
}

Prop::Prop(const string & relativePath) : Prop()
{
	addComponent(new Anim_Model_Component(relativePath, &worldState));
	Update();
}

void Prop::Update()
{
	lock_guard<shared_mutex> write_guard(data_mutex);
	worldState.Update();
	auto qwe = (Anim_Model_Component*)Component_Manager::GetComponent(m_component_handles[0]);
}

bool Prop::shouldRender(const mat4 & PVMatrix)
{
	/*shared_lock<shared_mutex> read_guard(data_mutex);
	shared_lock<shared_mutex> model_read_guard(assetModel->m_mutex);
	Frustum frustum(PVMatrix * worldState.modelMatrix);

	if (frustum.AABBInFrustom(assetModel->bbox_min, assetModel->bbox_max))
		return true;*/

	return false;
}

void Prop::geometryPass() const
{
	
}
