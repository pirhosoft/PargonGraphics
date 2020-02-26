#pragma once

#include "Pargon/Graphics/Texture.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <d3d11.h>
#include <wrl.h>

namespace Pargon
{
	class DirectX11TextureHandle : public TextureHandle
	{
	public:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture2d;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Resource;
		Microsoft::WRL::ComPtr<ID3D11View> View;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture2dMs;

		int SampleCount = 1;
		unsigned int Width = 0;
		unsigned int Height = 0;
		unsigned int Channels = 0;

		auto Update(Texture* texture) -> bool override;
	};
}
