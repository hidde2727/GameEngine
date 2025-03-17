#ifndef ENGINE_ENTRYPOINT_H
#define ENGINE_ENTRYPOINT_H

#include "Core/Game.h"

namespace Engine {
	std::unique_ptr<Engine::Game> CreateApplication();
}

#endif