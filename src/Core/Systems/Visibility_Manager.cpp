#include "Systems\Visibility_Manager.h"
#include <algorithm>
#include <windows.h> 

static shared_mutex camera_list_mutex;
static vector<Camera*> camera_list;
static thread *thread_visibility_shadow;
static shared_mutex systemActiveMutex;
bool systemActive, systemPause;

void Visibility_Manager::statup()
{
	/*systemActive = true;
	systemPause = false;*/
	/*thread_visibility_shadow = new thread(&Visibility_Manager::Visibility_Checker);
	thread_visibility_shadow->detach();*/
}

void Visibility_Manager::shutdown()
{
	/*lock_guard<shared_mutex> system_active_lock_guard(systemActiveMutex);
	systemActive = false;*/

	/*if (thread_visibility_shadow->joinable())
		thread_visibility_shadow->join();

	delete thread_visibility_shadow;*/
}

void Visibility_Manager::pause() 
{ 
	/*lock_guard<shared_mutex> system_active_lock_guard(systemActiveMutex); 
	systemPause = true; */
}

void Visibility_Manager::resume() 
{ 
	/*lock_guard<shared_mutex> system_active_lock_guard(systemActiveMutex); 
	systemPause = false; */
}

void Visibility_Manager::RegisterViewer(Camera * camera)
{
	/*lock_guard<shared_mutex> cam_list_guard(camera_list_mutex);
	camera_list.push_back(camera);*/
}

void Visibility_Manager::UnRegisterViewer(Camera * camera)
{
	/*lock_guard<shared_mutex> cam_list_guard(camera_list_mutex);
	camera_list.erase(std::remove_if(begin(camera_list), end(camera_list), [camera](const auto *ref) {
		return (ref == camera);
	}), end(camera_list));	*/
}

void Visibility_Manager::DeRegisterGeometryFromViewers(Geometry *g)
{
	/*shared_lock<shared_mutex> cam_list_guard(camera_list_mutex);
	for each (auto *cam in camera_list) {
		for (auto & spot = begin(cam->GetVisibilityToken().visible_geometry); spot != end(cam->GetVisibilityToken().visible_geometry); spot++) {
			vector<Geometry*> &vec = (*spot).second;
			vec.erase(std::remove_if(begin(vec), end(vec), [g](auto *ref) {
				return (ref == g);
			}), end(vec));
		}		
	}*/
}

void Visibility_Manager::Visibility_Checker()
{
	/*bool keep_looping = true;
	while (keep_looping) {
		{
			shared_lock<shared_mutex> system_active_lock_guard(systemActiveMutex);
			keep_looping = systemActive;

			// Cycle through each viewing perspective
			const shared_lock<shared_mutex> cam_list_guard(camera_list_mutex);
			for each (auto *camera in camera_list) {
				shared_lock<shared_mutex> camera_read_guard(camera->getDataMutex());
				if (camera->shouldRender()) {
					const auto &camBuffer = camera->getCameraBuffer();
					const mat4 camPVMatrix = camBuffer.pMatrix * camBuffer.vMatrix;
					map<int, vector<Geometry*>> geometry_list;
					map<int, vector<Light*>> light_list;
					{
						const auto &geometryMap = ComponentFactory::GetComponentsByType("Anim_Model");
						shared_lock<shared_mutex> geometry_read_guard(ComponentFactory::GetDataLock());

						for (int x = 0; x < geometryMap.size(); ++x) {
							geometry_list.insert(pair<int, vector<Geometry*>>(x, vector<Geometry*>()));
							for each (auto *obj in geometryMap.at(x))
								if (obj->shouldRender(camPVMatrix))
									geometry_list.at(x).push_back(obj);
						}

						shared_lock<shared_mutex> light_read_guard(Lighting_Manager::GetDataLock());
						const auto &lightingMap = Lighting_Manager::GetAllLights();

						for (int x = 0; x < lightingMap.size(); ++x) {
							light_list.insert(pair<int, vector<Light*>>(x, vector<Light*>()));
							for each (auto *obj in lightingMap.at(x))
								if (obj->shouldRender(camPVMatrix))
									light_list.at(x).push_back(obj);
						}
					}
					camera_read_guard.unlock();
					camera_read_guard.release();
					lock_guard<shared_mutex> camera_write_guard(camera->getDataMutex());
					Visibility_Token &vis_token = camera->GetVisibilityToken();
					vis_token.visible_geometry = geometry_list;
					vis_token.visible_lights = light_list;
				}
			}
		}
		bool shouldSleep = false;
		do {
			Sleep(16.6);

			shared_lock<shared_mutex> system_active_lock_guard(systemActiveMutex);
			shouldSleep = systemPause;
		} while (shouldSleep);
	}*/
}