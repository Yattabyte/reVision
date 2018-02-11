#include "Systems\Animation\Animation.h"
#include "Systems\World\World.h"
#include "Utilities\EnginePackage.h"
#include "Entities\Components\Anim_Model_Component.h"

System_Animation::~System_Animation()
{
}

System_Animation::System_Animation()
{
}

void System_Animation::Initialize(EnginePackage * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;

		m_Initialized = true;
	}
}

void System_Animation::Update(const float & deltaTime)
{
	System_World *world = m_enginePackage->getSubSystem<System_World>("World");
	if (!world) return;
	auto &models = world->GetSpecificComponents<Anim_Model_Component>("Anim_Model");
	for each (auto model in models)
		model->animate(deltaTime);
}

void System_Animation::Update_Threaded(const float & deltaTime)
{
}
