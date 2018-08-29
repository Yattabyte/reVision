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
	std::unique_lock<std::shared_mutex> write_guard(m_mutex);
	if (m_callbacks.find(pointerID) != m_callbacks.end())
		m_callbacks.erase(m_callbacks.find(pointerID));	
}

bool Asset::existsYet() const
{ 
	if (!this)
		return false;
	std::shared_lock<std::shared_mutex> read_guard(m_mutex);
	return m_finalized;
}

void Asset::finalize(Engine * engine)
{
	std::unique_lock<std::shared_mutex> write_guard(m_mutex);
	m_finalized = true;

	// Notify Completion
	AssetManager & assetManager = engine->getAssetManager();
	for each (auto qwe in m_callbacks)
		assetManager.submitNotifyee(qwe.first, qwe.second);
}
