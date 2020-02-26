#pragma once

#include "Pargon/Graphics/Geometry.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <d3d11.h>
#include <wrl.h>

namespace Pargon
{
	class DirectX11GeometryHandle : public GeometryHandle
	{
	public:
		Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer;
		UINT Capacity = 0;
		UINT Binding;

		auto Update(Geometry* geometry) -> bool override;
	};
}
