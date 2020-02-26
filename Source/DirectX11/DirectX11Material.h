#pragma once

#include "Pargon/Graphics/Material.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <d3d11.h>
#include <wrl.h>

namespace Pargon
{
	class DirectX11MaterialHandle : public MaterialHandle
	{
	public:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerState;
		Microsoft::WRL::ComPtr<ID3D11BlendState> BlendState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilState;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> Layout;
		List<Microsoft::WRL::ComPtr<ID3D11SamplerState>> SamplerStates;

		auto Update(Material* material) -> bool override;
	};
}