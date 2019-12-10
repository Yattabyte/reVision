#include "Engine.h"


int main() noexcept
{
	Engine engine;

	while (!engine.shouldClose())
		engine.tick();

	return 1;
}