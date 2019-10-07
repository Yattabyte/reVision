#include "Engine.h"


int main()
{
	Engine engine;

	while (!(engine.shouldClose())) [[likely]]
		engine.tick();

	return 1;
}