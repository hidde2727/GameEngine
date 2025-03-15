#include "core/Entry.h"

namespace Engine {
	extern Game* CreateApplication();

	int EngineMain(int c, char* v[]) {
		Engine::Game* game = Engine::CreateApplication();
		return game->Run();
	}
}

int main(int c, char* v[]) {
	return Engine::EngineMain(c, v);
}