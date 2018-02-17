#include "Systems\Animation\Animation.h"
#include "Systems\World\World.h"
#include "Utilities\EnginePackage.h"
#include "Entities\Components\Anim_Model_Component.h"


void System_Animation::initialize(EnginePackage * enginePackage)
{
	if (!m_Initialized) {
		m_enginePackage = enginePackage;

		m_Initialized = true;
	}
}

void System_Animation::update(const float & deltaTime)
{
	System_World *world = m_enginePackage->getSubSystem<System_World>("World");
	if (!world) return;
	auto &models = world->getSpecificComponents<Anim_Model_Component>("Anim_Model");
	for each (auto model in models)
		model->animate(deltaTime);
}