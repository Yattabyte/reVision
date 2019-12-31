#include "Engine.h"


int main()
{
	Engine engine;

	while (!engine.shouldClose())
		engine.tick();

	return 1;
}