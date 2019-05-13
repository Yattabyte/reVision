#include "Engine.h"


int main()
{
	{	// Create the engine;
		Engine engine;

		// Begin main thread
		while (!(engine.shouldClose()))
			engine.tick();
	}	// Destroy the engine before pausing + exit
#ifdef NDEBUG
	system("pause");
#endif
	exit(1);
}