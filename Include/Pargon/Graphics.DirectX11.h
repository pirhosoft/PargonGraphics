#pragma once

#include "Pargon/Graphics.h"
#include <memory>

namespace Pargon
{
	auto CreateDirectX11Renderer() -> std::unique_ptr<Renderer>;
}
