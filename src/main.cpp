#include "Engine.h"
#include "Systems\World\Camera.h"
#include <thread>


static void Tick_Primary(Engine &engine) {
	engine.tick();
}

static void Tick_Secondary(Engine &engine) {
	engine.tickThreaded();
}

int main()
{	
	Engine engine;
	thread *m_UpdaterThread;
	Camera * camera;

	if (!engine.initialize())
		exit(-1);

	camera = engine.getCamera();
	m_UpdaterThread = new thread(Tick_Secondary, engine);
	m_UpdaterThread->detach();

	while (!(engine.shouldClose())) {
		camera->bind();
		Tick_Primary(engine);
	}

	if (m_UpdaterThread->joinable())
		m_UpdaterThread->join();
	delete m_UpdaterThread;
	engine.shutdown();
}