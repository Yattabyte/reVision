#include "Engine.h"
#include "Systems\World\Camera.h"
#include <thread>


int main()
{	
	Engine engine;
	thread *m_UpdaterThread;
	Camera * camera;

	if (!engine.initialize())
		exit(-1);

	camera = engine.getCamera();
	m_UpdaterThread = new thread(&Engine::tickThreaded, &engine);
	m_UpdaterThread->detach();

	while (!(engine.shouldClose())) {
		camera->bind();
		engine.tick();
	}

	/*if (m_UpdaterThread->joinable())
		m_UpdaterThread->join();
	delete m_UpdaterThread;*/
	engine.shutdown();
}