#include "Engine.h"


int main()
{	
	// Create the engine;
	Engine engine;

	// Begin main thread
	while (!(engine.shouldClose())) 
		engine.tick();
}