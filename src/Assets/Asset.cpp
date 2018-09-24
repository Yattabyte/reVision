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
	// Exit early if this points to nothing
	if (!this)
		return false;

	// Check if we're finalized
	if (!(m_finalized.load()))
		return false;
	
	// Check if we have a fence
	if (m_fence) {
		// Check if the fence has passed
		const GLenum state = glClientWaitSync(m_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
		if (state == GL_SIGNALED || state == GL_ALREADY_SIGNALED || state == GL_CONDITION_SATISFIED) {
			// Delete fence so we can skip these 2 branches next time
			glDeleteSync(m_fence);
			m_fence = 0;
		}
	}
	return true;	
}

void Asset::finalize(Engine * engine)
{
	m_finalized = true;

	// Notify Completion
	AssetManager & assetManager = engine->getAssetManager();
	for each (auto qwe in m_callbacks)
		assetManager.submitNotifyee(qwe.first, qwe.second);
}
