#include "dt_Engine.h"
#include "Systems\World\Camera.h"
#include <thread>


static void Tick_Primary(dt_Engine &engine) {
	engine.tick();
}

static void Tick_Secondary(dt_Engine &engine) {
	engine.tickThreaded();
}

int main()
{	
	dt_Engine engine;
	thread *m_UpdaterThread;
	Camera * camera;

	if (!engine.initialize())
		exit(-1);

	camera = engine.getCamera();
	m_UpdaterThread = new thread(Tick_Secondary, engine);
	m_UpdaterThread->detach();

	while (!(engine.shouldClose())) {
		camera->Bind();
		//camera->update();
		Tick_Primary(engine);
	}

	if (m_UpdaterThread->joinable())
		m_UpdaterThread->join();
	delete m_UpdaterThread;
	engine.shutdown();
}