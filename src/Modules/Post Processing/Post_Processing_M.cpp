#include "Modules/Post Processing/Post_Processing_M.h"
#include "Modules/Graphics/Graphics_M.h"

/* Post Processing Techniques Used */
#include "Modules/Post Processing/Effects/To_Screen.h"
#include "Modules/Post Processing/Effects/Frametime_Counter.h"
#include "Modules/Post Processing/Effects/LoadingIndicator.h"


void Post_Processing_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: Post-Processing...");
	m_fxTechs.push_back(new To_Screen(m_engine));
	m_fxTechs.push_back(new Frametime_Counter(m_engine));
	m_fxTechs.push_back(new LoadingIndicator(m_engine));
}

void Post_Processing_Module::frameTick(const float & deltaTime)
{
	// Apply each effect
	for each (auto *tech in m_fxTechs)
		if (tech->isEnabled())
			tech->applyTechnique(deltaTime);
}
