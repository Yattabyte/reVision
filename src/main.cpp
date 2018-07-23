#include "Engine.h"
#include "Systems\World\Camera.h"
#include <future>
#include <thread>


int main()
{	
	// Create our objects
	Engine engine;
	Camera * camera;

	// Begin Initialization
	if (!engine.initialize())
		exit(-1);

	// Begin threaded operations
	std::promise<void> exitSignal;
	std::future<void> exitObject = exitSignal.get_future();
	std::thread m_UpdaterThread(&Engine::tickThreaded, &engine, std::move(exitObject));
	m_UpdaterThread.detach();

	// Begin main thread
	while (!(engine.shouldClose())) {
		engine.getCamera()->bind();
		engine.tick();
	}

	// Shutdown
	exitSignal.set_value();
	engine.shutdown();
}