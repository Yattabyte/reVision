#include "Modules/Post Processing/Post_Processing_M.h"
#include "Modules/Graphics/Graphics_M.h"

/* Post Processing Techniques Used */
#include "Modules/Post Processing/Effects/Bloom.h"
#include "Modules/Post Processing/Effects/HDR.h"
#include "Modules/Post Processing/Effects/FXAA.h"
#include "Modules/Post Processing/Effects/To_Screen.h"
#include "Modules/Post Processing/Effects/Frametime_Counter.h"


void Post_Processing_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_fxTechs.push_back(new Bloom(m_engine, m_engine->getModule_Graphics().getLightingFBOID(), m_engine->getModule_Graphics().getLightingTexID()));
	m_fxTechs.push_back(new HDR(m_engine));
	m_fxTechs.push_back(new FXAA(m_engine));
	m_fxTechs.push_back(new To_Screen(m_engine));
	m_fxTechs.push_back(new Frametime_Counter(m_engine));
}

void Post_Processing_Module::frameTick(const float & deltaTime)
{
	// Apply each effect
	for each (auto *tech in m_fxTechs)
		if (tech->isEnabled())
			tech->applyEffect(deltaTime);
}
