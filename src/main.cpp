#include "Engine.h"
#include <future>
#include <thread>

#include "ECS/Components/Transform_C.h"
#include "ECS/Components/Prop_C.h"
#include "ECS/Components/Skeleton_C.h"
#include "ECS/Components/Skybox_C.h"
#include "ECS/Components/LightDirectional_C.h"


int main()
{	
	// Create the engine;
	Engine engine;

	// Begin threaded operations
	std::promise<void> exitSignal;
	std::future<void> exitObject = exitSignal.get_future();
	std::thread m_UpdaterThread(&Engine::tickThreaded, &engine, std::move(exitObject));
	m_UpdaterThread.detach();

	// Begin main thread
	while (!(engine.shouldClose())) 
		engine.tick();	

	// Shutdown
	exitSignal.set_value();
}