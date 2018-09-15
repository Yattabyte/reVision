#include "Assets\Asset.h"
#include "Engine.h"
#include <algorithm>


Asset::Asset(const std::string & filename) : m_filename(filename) {}

std::string Asset::getFileName() const
{
	return m_filename;
}

void Asset::setFileName(const std::string & fn)
{
	m_filename = fn;
}

void Asset::removeCallback(void * pointerID) 
{
	if (m_callbacks.find(pointerID) != m_callbacks.end())
		m_callbacks.erase(m_callbacks.find(pointerID));	
}

bool Asset::existsYet() const
{ 
	if (!this)
		return false;
	return m_finalized;
}

void Asset::finalize(Engine * engine)
{
	m_finalized = true;

	// Notify Completion
	AssetManager & assetManager = engine->getAssetManager();
	for each (auto qwe in m_callbacks)
		assetManager.submitNotifyee(qwe.first, qwe.second);
}
