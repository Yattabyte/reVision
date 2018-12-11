#include "Engine.h"


int main()
{	
	{	// Create the engine;
		Engine engine;

		// Begin main thread
		while (!(engine.shouldClose()))
			engine.tick();
	}	// Destroy the engine before pausing + exit
	system("pause");
	exit(1);
}