#include "Pargon/Application/Log.h"
#include "Pargon/Graphics/GraphicsDevice.h"
#include "DirectX11/DirectX11Renderer.h"
#include "DirectX11/DirectX11Texture.h"

using namespace Pargon;

namespace
{
	auto GetColorTargetDescription(unsigned int width, unsigned int height) -> D3D11_TEXTURE2D_DESC
	{
		D3D11_TEXTURE2D_DESC description;

		description.Width = width;
		description.Height = height;
		description.MipLevels = 1;
		description.ArraySize = 1;
		description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		description.SampleDesc.Count = 1;
		description.SampleDesc.Quality = 0;
		description.Usage = D3D11_USAGE_DEFAULT;
		description.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		description.CPUAccessFlags = 0;
		description.MiscFlags = 0;

		return description;
	}

	auto GetMultisampleColorTargetDescription(unsigned int width, unsigned int height, unsigned int sampleCount) -> D3D11_TEXTURE2D_DESC
	{
		D3D11_TEXTURE2D_DESC description;

		description.Width = width;
		description.Height = height;
		description.MipLevels = 1;
		description.ArraySize = 1;
		description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		description.SampleDesc.Count = sampleCount;
		description.SampleDesc.Quality = 0;
		description.Usage = D3D11_USAGE_DEFAULT;
		description.BindFlags = D3D11_BIND_RENDER_TARGET;
		description.CPUAccessFlags = 0;
		description.MiscFlags = 0;

		return description;
	}

	auto GetDepthStencilTargetDescription(unsigned int width, unsigned int height) -> D3D11_TEXTURE2D_DESC
	{
		D3D11_TEXTURE2D_DESC description;

		description.Width = width;
		description.Height = height;
		description.MipLevels = 1;
		description.ArraySize = 1;
		description.Format = DXGI_FORMAT_R24G8_TYPELESS;
		description.SampleDesc.Count = 1;
		description.SampleDesc.Quality = 0;
		description.Usage = D3D11_USAGE_DEFAULT;
		description.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		description.CPUAccessFlags = 0;
		description.MiscFlags = 0;

		return description;
	}

	auto GetMultisampleDepthStencilTargetDescription(unsigned int width, unsigned int height, unsigned int sampleCount) -> D3D11_TEXTURE2D_DESC
	{
		D3D11_TEXTURE2D_DESC description;

		description.Width = width;
		description.Height = height;
		description.MipLevels = 1;
		description.ArraySize = 1;
		description.Format = DXGI_FORMAT_R24G8_TYPELESS;
		description.SampleDesc.Count = sampleCount;
		description.SampleDesc.Quality = 0;
		description.Usage = D3D11_USAGE_DEFAULT;
		description.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		description.CPUAccessFlags = 0;
		description.MiscFlags = 0;

		return description;
	}

	auto GetColorBufferDescription(unsigned int width, unsigned int height, bool generateMips, bool dynamic) -> D3D11_TEXTURE2D_DESC
	{
		D3D11_TEXTURE2D_DESC description;

		description.Width = width;
		description.Height = height;
		description.MipLevels = generateMips ? 0 : 1;
		description.ArraySize = 1;
		description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		description.SampleDesc.Count = 1;
		description.SampleDesc.Quality = 0;
		description.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		description.BindFlags = D3D11_BIND_SHADER_RESOURCE;// mipmapCount == Texture::MipmapNone ? D3D11_BIND_SHADER_RESOURCE : (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
		description.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
		description.MiscFlags = 0;//mipmapCount == Texture::MipmapNone ? 0 : D3D11_RESOURCE_MISC_GENERATE_MIPS;

		return description;
	}

	auto GetTextureDescription(TextureFormat format, unsigned int width, unsigned int height, bool generateMips, bool dynamic) -> D3D11_TEXTURE2D_DESC
	{
		switch (format)
		{
		case TextureFormat::ColorBuffer4x8: return GetColorBufferDescription(width, height, generateMips, dynamic);
		case TextureFormat::ColorTarget4x8: return GetColorTargetDescription(width, height);
		case TextureFormat::DepthStencilTarget24_8: return GetDepthStencilTargetDescription(width, height);
		}

		return GetColorBufferDescription(width, height, generateMips, dynamic);
	}

	auto GetLayerData(BufferView base, SequenceView<Buffer> mipmaps, TextureFormat format, unsigned int width, unsigned int height, unsigned int depth) -> List<D3D11_SUBRESOURCE_DATA>
	{
		if (format != TextureFormat::ColorBuffer4x8)
			return {};

		List<D3D11_SUBRESOURCE_DATA> layerData;

		{
			auto& data = layerData.Increment();

			data.pSysMem = base.begin();
			data.SysMemPitch = width * depth;
			data.SysMemSlicePitch = 0;

			width >>= 1;
			height >>= 1;
		}

		for (auto i = 0; i < mipmaps.Count(); i++)
		{
			auto& data = layerData.Increment();

			data.pSysMem = mipmaps.Item(i).begin();
			data.SysMemPitch = width * depth;
			data.SysMemSlicePitch = 0;

			width >>= 1;
			height >>= 1;
		}

		return layerData;
	}

	auto GetTexture(ID3D11Device* device, const Texture& texture, unsigned int width, unsigned int height, ID3D11Texture2D** texture2d, ID3D11Texture2D** texture2dMs, int sampleCount) -> bool
	{
		auto description = GetTextureDescription(texture.Format(), width, height, false, texture.Storage() == GraphicsStorage::StreamedToGpu);
		auto result = device->CreateTexture2D(&description, nullptr, texture2d);

		if (sampleCount > 1)
		{
			if (texture.Format() == TextureFormat::ColorTarget4x8)
			{
				auto msDescription = GetMultisampleColorTargetDescription(width, height, sampleCount);
				device->CreateTexture2D(&msDescription, nullptr, texture2dMs);
			}
			else if (texture.Format() == TextureFormat::DepthStencilTarget24_8)
			{
				auto msDescription = GetMultisampleDepthStencilTargetDescription(width, height, sampleCount);
				device->CreateTexture2D(&msDescription, nullptr, texture2dMs);
			}
		}

		if (FAILED(result))
		{
			//Log::Write("failed to create the texture");
			return false;
		}

		return true;
	}

	auto GetResource(TextureFormat format, ID3D11Device* device, ID3D11Texture2D* texture2d, ID3D11ShaderResourceView** resource) -> bool
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC description;
		description.Format = format == TextureFormat::DepthStencilTarget24_8 ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R8G8B8A8_UNORM;
		description.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		description.Texture2D.MostDetailedMip = 0;
		description.Texture2D.MipLevels = -1;

		auto result = device->CreateShaderResourceView(texture2d, &description, resource);

		if (FAILED(result))
		{
			//Log::Write("failed to create the resource");
			return false;
		}

		return true;
	}

	auto GetColorTargetView(ID3D11Device* device, ID3D11Texture2D* texture2d, bool multisampled) -> Microsoft::WRL::ComPtr<ID3D11RenderTargetView>
	{
		ID3D11RenderTargetView* view;

		D3D11_RENDER_TARGET_VIEW_DESC description;
		description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		description.ViewDimension = multisampled ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
		description.Texture2D.MipSlice = 0;

		auto result = device->CreateRenderTargetView(texture2d, &description, std::addressof(view));

		if (FAILED(result))
		{
			//Log::Write("failed to create the view");
			return nullptr;
		}

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pointer = view;
		view->Release();
		return pointer;
	}

	auto GetDepthStencilTargetView(ID3D11Device* device, ID3D11Texture2D* texture2d, bool multisampled) -> Microsoft::WRL::ComPtr<ID3D11DepthStencilView>
	{
		if (texture2d == nullptr)
			return nullptr;

		ID3D11DepthStencilView* view;

		D3D11_DEPTH_STENCIL_VIEW_DESC description;
		description.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		description.ViewDimension = multisampled ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
		description.Flags = 0;
		description.Texture2D.MipSlice = 0;

		auto result = device->CreateDepthStencilView(texture2d, &description, std::addressof(view));

		if (FAILED(result))
		{
			//Log::Write("failed to create the view");
			return nullptr;
		}

		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pointer = view;
		view->Release();
		return pointer;
	}

	auto GetView(TextureFormat format, ID3D11Device* device, ID3D11Texture2D* texture2d, bool multisampled) -> Microsoft::WRL::ComPtr<ID3D11View>
	{
		Microsoft::WRL::ComPtr<ID3D11View> view;

		switch (format)
		{
		case TextureFormat::ColorTarget4x8:
		{
			auto colorView = GetColorTargetView(device, texture2d, multisampled);
			colorView.As(std::addressof(view));
			break;
		}
		case TextureFormat::DepthStencilTarget24_8:
		{
			auto depthView = GetDepthStencilTargetView(device, texture2d, multisampled);
			depthView.As(std::addressof(view));
			break;
		}
		}

		return view;
	}
}

auto DirectX11TextureHandle::Update(Texture* texture) -> bool
{
	auto renderer = static_cast<DirectX11Renderer*>(texture->Graphics().Renderer());

	if (texture->Format() != TextureFormat::ColorBuffer4x8 || !Texture2d || !Resource || Width != texture->Size().Width || Height != texture->Size().Height || Channels != texture->Depth() || SampleCount != texture->SampleCount())
	{
		Width = texture->Size().Width;
		Height = texture->Size().Height;
		Channels = texture->Depth();
		SampleCount = texture->SampleCount();

		if (Texture2d) Texture2d.Reset();
		if (Texture2dMs) Texture2dMs.Reset();
		if (Resource) Resource.Reset();
		if (View) View.Reset();

		auto success = ::GetTexture(renderer->Device.Get(), *texture, Width, Height, Texture2d.GetAddressOf(), Texture2dMs.GetAddressOf(), SampleCount);

		if (success)
			success = ::GetResource(texture->Format(), renderer->Device.Get(), Texture2d.Get(), Resource.GetAddressOf());

		if (success)
			View = ::GetView(texture->Format(), renderer->Device.Get(), SampleCount > 1 ? Texture2dMs.Get() : Texture2d.Get(), SampleCount > 1);
	}

	if (texture->Format() == TextureFormat::ColorBuffer4x8)
	{
		auto reservations = texture->Reservations();

		for (auto& reservation : reservations)
		{
			auto data = reservation.Data.begin();

			D3D11_BOX box;
			box.left = reservation.Location.X;
			box.top = reservation.Location.Y;
			box.front = 0;
			box.right = reservation.Location.X + reservation.Size.Width;
			box.bottom = reservation.Location.Y + reservation.Size.Height;
			box.back = 1;
			renderer->Context->UpdateSubresource(Texture2d.Get(), 0, &box, reservation.Data.begin(), reservation.Pitch, reservation.Pitch * reservation.Size.Height);
		}
	}

	return true;
}