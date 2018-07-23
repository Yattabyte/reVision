#include "Managers\AssetManager.h"
#include <minmax.h>

/** Begin Asset Includes **/
#include "Assets/Asset.h"
#include "Assets/Asset_Collider.h"
#include "Assets/Asset_Config.h"
#include "Assets/Asset_Cubemap.h"
#include "Assets/Asset_Material.h"
#include "Assets/Asset_Model.h"
#include "Assets/Asset_Primitive.h"
#include "Assets/Asset_Shader.h"
#include "Assets\Asset_Shader_Geometry.h"
#include "Assets/Asset_Shader_Pkg.h"
#include "Assets/Asset_Texture.h"
/** End Asset Includes **/

 
AssetManager::~AssetManager()
{
	/** @todo destructor */

	{
		std::unique_lock<std::shared_mutex> worker_writeGuard(m_workerNotificationMutex);
		m_running = false;
	}
	for each (auto thread in m_Workers) {
		if (thread->joinable())
			thread->join();
		delete thread;
	}
}

AssetManager::AssetManager(Engine * engine) : m_engine(engine)
{
	// Default Parameters
	m_running = true;

	// Register asset creators
	registerAssetCreator(typeid(Shared_Asset_Collider).name(), std::function<void(Engine*, Shared_Asset_Collider&, const std::string&, const bool &)>(Asset_Collider::Create));
	registerAssetCreator(typeid(Shared_Asset_Config).name(), std::function<void(Engine*, Shared_Asset_Config&, const std::string&, const std::vector<std::string> &, const bool &)>(Asset_Config::Create));
	registerAssetCreator(typeid(Shared_Asset_Cubemap).name(), std::function<void(Engine*, Shared_Asset_Cubemap&, const std::string&, const bool &&)>(Asset_Cubemap::Create));
	registerAssetCreator(typeid(Shared_Asset_Material).name(), std::function<void(Engine*, Shared_Asset_Material&, const std::string&, const bool &, const std::string(&)[MAX_PHYSICAL_IMAGES])>(Asset_Material::Create));
	registerAssetCreator(typeid(Shared_Asset_Model).name(), std::function<void(Engine*, Shared_Asset_Model&, const std::string&, const bool &)>(Asset_Model::Create));
	registerAssetCreator(typeid(Shared_Asset_Primitive).name(), std::function<void(Engine*, Shared_Asset_Primitive&, const std::string&, const bool &)>(Asset_Primitive::Create));
	registerAssetCreator(typeid(Shared_Asset_Shader).name(), std::function<void(Engine*, Shared_Asset_Shader&, const std::string&, const bool &)>(Asset_Shader::Create));
	registerAssetCreator(typeid(Shared_Asset_Shader_Geometry).name(), std::function<void(Engine*, Shared_Asset_Shader_Geometry&, const std::string&, const bool &)>(Asset_Shader_Geometry::Create));
	registerAssetCreator(typeid(Shared_Asset_Shader_Pkg).name(), std::function<void(Engine*, Shared_Asset_Shader_Pkg&, const std::string&, const bool &)>(Asset_Shader_Pkg::Create));
	registerAssetCreator(typeid(Shared_Asset_Texture).name(), std::function<void(Engine*, Shared_Asset_Texture&, const std::string&, const GLuint &, const bool &, const bool &, const bool &)>(Asset_Texture::Create));
	
	// Initialize worker threads
	const unsigned int maxThreads = std::min(4u, std::min(ASSETMANAGER_MAX_THREADS, std::thread::hardware_concurrency()));
	for (unsigned int x = 0; x < maxThreads; ++x) {
		std::thread * workerThread = new std::thread(&AssetManager::initializeOrders, this);
		workerThread->detach();
		m_Workers.push_back(workerThread);
	}
}

void AssetManager::initializeOrders()
{
	while (true) {
		// Check if worker should shutdown
		std::shared_lock<std::shared_mutex> worker_readGuard(m_workerNotificationMutex);
		if (!m_running)
			return;

		// Start reading work orders
		std::unique_lock<std::shared_mutex> worker_writeGuard(m_Mutex_Workorders);
		if (m_Work_toStart.size()) {
			// Remove front of queue
			Asset_Work_Order * workOrder = m_Work_toStart.front();
			m_Work_toStart.pop_front();
			worker_writeGuard.unlock();
			worker_writeGuard.release();

			// Initialize asset
			workOrder->start();

			// Add to finalization queue
			std::unique_lock<std::shared_mutex> new_manager_writeGuard(m_Mutex_Workorders);
			m_Work_toFinish.push_back(workOrder);
		}
	}
}

void AssetManager::finalizeOrders() 
{
	std::unique_lock<std::shared_mutex> worker_writeGuard(m_Mutex_Workorders);

	// Quit early
	if (!m_Work_toFinish.size())
		return;

	// Remove front of queue
	Asset_Work_Order * workOrder = m_Work_toFinish.front();
	m_Work_toFinish.pop_front();
	worker_writeGuard.unlock();
	worker_writeGuard.release();

	// Finalize asset
	workOrder->finish();

	// Delete work order
	delete workOrder;	
}

void AssetManager::submitNotifyee(void * pointerID, const std::function<void()> & callBack)
{
	m_notifyees.push_back(std::make_pair(pointerID,callBack));
}

void AssetManager::removeNotifyee(void * pointerID)
{
	m_notifyees.erase(std::remove_if(begin(m_notifyees), end(m_notifyees), [pointerID](const std::pair<void*, std::function<void()>> & notifyee) {
		return (notifyee.first == pointerID);
	}), end(m_notifyees));
}

void AssetManager::notifyObservers()
{
	for each (const auto & notifyee in m_notifyees)
		notifyee.second();
	m_notifyees.clear();
}