#include "Systems\Animation\Animation.h"
#include "ECS\Components\Model_Animated.h"
#include "Engine.h"


void System_Animation::initialize(Engine * engine)
{
	if (!m_Initialized) {
		m_engine = engine;

		m_Initialized = true;
	}
}

void System_Animation::update(const float & deltaTime)
{
	if (m_Initialized) {
		const auto &models = m_engine->getECS().getSpecificComponents<Model_Animated_C>(Model_Animated_C::GetName());
		for each (auto model in models)
			model->animate(deltaTime);
	}
}