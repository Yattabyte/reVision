#include "Systems\World\Animator.h"
#include "ECS\Components\Model_Animated.h"
#include "Systems\World\World.h"


void Animator::animate(const float & deltaTime)
{
	const auto &models = m_world->getSpecificComponents<Model_Animated_C>(Model_Animated_C::GetName());
	for each (auto model in models)
		model->animate(deltaTime);
}