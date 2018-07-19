#include "Engine.h"
#include "Systems\World\Camera.h"
#include <thread>


int main()
{	
	// Create our objects
	Engine engine;
	std::thread *m_UpdaterThread;
	Camera * camera;

	// Begin Initialization
	if (!engine.initialize())
		exit(-1);

	// Begin threaded operations
	m_UpdaterThread = new std::thread(&Engine::tickThreaded, &engine);
	m_UpdaterThread->detach();

	// Begin main thread
	while (!(engine.shouldClose())) {
		engine.getCamera()->bind();
		engine.tick();
	}

	// Shutdown
	if (m_UpdaterThread->joinable())
		m_UpdaterThread->join();
	delete m_UpdaterThread;
	engine.shutdown();
}