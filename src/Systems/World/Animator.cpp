#include "Systems\World\Animator.h"
#include "Systems\World\ECS\Components\Anim_Model_Component.h"
#include "Systems\World\World.h"


void Animator::animate(const float & deltaTime)
{
	const auto &models = m_world->getSpecificComponents<Anim_Model_Component>("Anim_Model");
	for each (auto model in models)
		model->animate(deltaTime);
}