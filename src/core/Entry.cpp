#include "core/Entry.h"

namespace Engine {
	extern std::unique_ptr<Engine::Game> CreateApplication();

	int EngineMain(int c, char* v[]) {
		std::unique_ptr<Engine::Game> game = Engine::CreateApplication();
		return game->Run();
	}
}

int main(int c, char* v[]) {
	return Engine::EngineMain(c, v);
}