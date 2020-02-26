#include "Pargon/Application/Log.h"
#include "DirectX11/DirectX11Renderer.h"
#include "DirectX11/DirectX11Geometry.h"
#include "DirectX11/DirectX11Material.h"
#include "DirectX11/DirectX11Texture.h"
#include "Pargon/Application.Win32.h"
#include "Pargon/Graphics.DirectX11.h"

#include <D3D11_1.h>
#include <D3Dcompiler.h>

using namespace Pargon;

auto Pargon::CreateDirectX11Renderer() -> std::unique_ptr<Renderer>
{
	return std::make_unique<DirectX11Renderer>();
}

DirectX11Renderer::~DirectX11Renderer()
{
	CoUninitialize();
}

auto DirectX11Renderer::CompileShader(StringView content, Material& material) -> ShaderCompilationResult
{
	ShaderCompilationResult result;

	auto flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_ALL_RESOURCES_BOUND | D3DCOMPILE_ENABLE_STRICTNESS;

	Microsoft::WRL::ComPtr<ID3DBlob> vertexErrors;
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
	auto hr = D3DCompile(content.begin(), content.Length(), material.Identifier().begin(), nullptr, nullptr, "VertexMain", "vs_4_0", flags, 0, &vertexShader, &vertexErrors);
	if (FAILED(hr))
	{
		auto errors = std::string(reinterpret_cast<const char*>(vertexErrors->GetBufferPointer()), vertexErrors->GetBufferSize());
		result.Errors.Add({ errors.data(), static_cast<int>(errors.length()) });
	}

	Microsoft::WRL::ComPtr<ID3DBlob> pixelErrors;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
	hr = D3DCompile(content.begin(), content.Length(), material.Identifier().begin(), nullptr, nullptr, "FragmentMain", "ps_4_0", flags, 0, &pixelShader, &pixelErrors);
	if (FAILED(hr))
	{
		auto errors = std::string(reinterpret_cast<const char*>(pixelErrors->GetBufferPointer()), pixelErrors->GetBufferSize());
		result.Errors.Add({ errors.data(), static_cast<int>(errors.length()) });
	}

	if (result.Errors.IsEmpty())
	{
		material.VertexShader = { reinterpret_cast<uint8_t*>(vertexShader->GetBufferPointer()), static_cast<int>(vertexShader->GetBufferSize()) };
		material.FragmentShader = { reinterpret_cast<uint8_t*>(pixelShader->GetBufferPointer()), static_cast<int>(pixelShader->GetBufferSize()) };
	}

	result.Identifier = material.Identifier();
	result.Success = result.Errors.IsEmpty();
	return result;
}

void DirectX11Renderer::Setup(Application& application, RendererInformation& information)
{
	CreateDevice(true); // TODO

	information.SupportedSampleCounts = GetAvailableSampleCounts();

	RECT rect;
	auto data = application.Data();

	GetClientRect(data->Window(), &rect);
	CreateSwapChain(nullptr, data->Window(), rect.right, rect.bottom);
}

auto DirectX11Renderer::CreateGeometryHandle() -> std::unique_ptr<GeometryHandle>
{
	return std::make_unique<DirectX11GeometryHandle>();
}

auto DirectX11Renderer::CreateMaterialHandle() -> std::unique_ptr<MaterialHandle>
{
	return std::make_unique<DirectX11MaterialHandle>();
}

auto DirectX11Renderer::CreateTextureHandle() -> std::unique_ptr<TextureHandle>
{
	return std::make_unique<DirectX11TextureHandle>();
}

void DirectX11Renderer::BeginFrame()
{
}

void DirectX11Renderer::SetRenderTarget(Texture* texture, int slot)
{
	if (slot > _currentRenderTargets.Count())
		return;

	_currentRenderTargets.SetCount(slot, nullptr);

	if (texture == nullptr)
	{
		_currentRenderTargets.Add(RenderTarget.Get());
		_currentViewport = Viewport;
	}
	else
	{
		auto textureHandle = texture->Handle<DirectX11TextureHandle>();
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> view;

		textureHandle->View.As(std::addressof(view));

		if (view)
		{
			_currentRenderTargets.Add(view.Get());

			_currentViewport.TopLeftX = 0.0f;
			_currentViewport.TopLeftY = 0.0f;
			_currentViewport.Width = static_cast<float>(texture->Size().Width);
			_currentViewport.Height = static_cast<float>(texture->Size().Height);
			_currentViewport.MinDepth = 0.0f;
			_currentViewport.MaxDepth = 1.0f;
		}
	}

	Context->OMSetRenderTargets(_currentRenderTargets.Count(), _currentRenderTargets.begin(), _currentDepthStencilTarget);
	Context->RSSetViewports(1, std::addressof(_currentViewport));
}

void DirectX11Renderer::ClearColorTarget(float r, float g, float b, float a)
{
	for (auto target : _currentRenderTargets)
	{
		float floats[] = { r, g, b, a };
		Context->ClearRenderTargetView(target, floats);
	}
}

void DirectX11Renderer::SetDepthStencilTarget(Texture* texture)
{
	if (texture == nullptr)
	{
		_currentDepthStencilTarget = nullptr;
	}
	else
	{
		auto textureHandle = texture->Handle<DirectX11TextureHandle>();
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> view;

		textureHandle->View.As(std::addressof(view));
		_currentDepthStencilTarget = view.Get();
	}

	Context->OMSetRenderTargets(_currentRenderTargets.Count(), _currentRenderTargets.begin(), _currentDepthStencilTarget);
	Context->RSSetViewports(1, std::addressof(_currentViewport));
}

void DirectX11Renderer::ClearDepthAndStencilTarget(float depth, int stencil)
{
	if (_currentDepthStencilTarget != nullptr)
		Context->ClearDepthStencilView(_currentDepthStencilTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
}

void DirectX11Renderer::SetClippingRectangle(float x, float y, float width, float height)
{
	D3D11_RECT rectangle;

	rectangle.left = static_cast<LONG>(std::round(x > 0.0f ? x : 0.0f));
	rectangle.right = static_cast<LONG>(std::round(width > 0.0f ? x + width : Viewport.Width));
	rectangle.top = static_cast<LONG>(std::round(y > 0.0f ? Viewport.Height - y - height : 0.0f));
	rectangle.bottom = static_cast<LONG>(std::round(height > 0.0f ? Viewport.Height - y : Viewport.Height));

	Context->RSSetScissorRects(1, std::addressof(rectangle));
}

void DirectX11Renderer::SetMaterial(Material* material)
{
	if (material != nullptr)
	{
		auto materialHandle = material->Handle<DirectX11MaterialHandle>();

		if (materialHandle->VertexShader && materialHandle->PixelShader && materialHandle->Layout)
		{
			Context->VSSetShader(materialHandle->VertexShader.Get(), nullptr, 0);
			Context->PSSetShader(materialHandle->PixelShader.Get(), nullptr, 0);
			Context->IASetInputLayout(materialHandle->Layout.Get());
		}

		if (materialHandle->RasterizerState)
			Context->RSSetState(materialHandle->RasterizerState.Get());

		if (materialHandle->BlendState)
			Context->OMSetBlendState(materialHandle->BlendState.Get(), nullptr, 0xFFFFFFFF);

		if (materialHandle->DepthStencilState)
			Context->OMSetDepthStencilState(materialHandle->DepthStencilState.Get(), material->StencilOptions.ReferenceValue);

		if (!materialHandle->SamplerStates.IsEmpty())
			Context->PSSetSamplers(0, materialHandle->SamplerStates.Count(), materialHandle->SamplerStates.First().GetAddressOf());
	}
}

void DirectX11Renderer::SetTexture(Texture* texture, int slot)
{
	ID3D11ShaderResourceView* clear[1] = { nullptr };

	if (texture == nullptr)
	{
		Context->PSSetShaderResources(slot, 1, clear);
	}
	else
	{
		auto textureHandle = texture->Handle<DirectX11TextureHandle>();

		if (textureHandle->SampleCount > 1)
			Context->ResolveSubresource(textureHandle->Texture2d.Get(), 0, textureHandle->Texture2dMs.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);

		Context->PSSetShaderResources(slot, 1, textureHandle->Resource.GetAddressOf());
	}
}

namespace
{
	auto GetTopology(GeometryTopology topology) -> D3D_PRIMITIVE_TOPOLOGY
	{
		switch (topology)
		{
		case GeometryTopology::PointList: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case GeometryTopology::TriangleList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case GeometryTopology::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case GeometryTopology::LineList: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		}

		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}

void DirectX11Renderer::SetVertexBuffer(Geometry* geometry, std::size_t vertexSize)
{
	auto offset = 0u;
	auto size = static_cast<UINT>(vertexSize);

	if (geometry == nullptr)
	{
		ID3D11Buffer* vertexBuffers[1] = { nullptr };
		Context->IASetVertexBuffers(0, 1, vertexBuffers, &size, &offset);
	}
	else
	{
		auto geometryHandle = geometry->Handle<DirectX11GeometryHandle>();

		if (geometryHandle->Buffer)
		{
			ID3D11Buffer* vertexBuffers[1] = { geometryHandle->Buffer.Get() };
			Context->IASetVertexBuffers(0, 1, vertexBuffers, &size, &offset);

			auto topology = GetTopology(geometry->Topology());
			Context->IASetPrimitiveTopology(topology);
		}
	}
}

void DirectX11Renderer::SetInstanceBuffer(Geometry* geometry, std::size_t vertexSize)
{
	auto offset = 0u;
	auto size = static_cast<UINT>(vertexSize);

	if (geometry == nullptr)
	{
		ID3D11Buffer* vertexBuffers[1] = { nullptr };
		Context->IASetVertexBuffers(1, 1, vertexBuffers, &size, &offset);
	}
	else
	{
		auto geometryHandle = geometry->Handle<DirectX11GeometryHandle>();

		if (geometryHandle->Buffer)
		{
			ID3D11Buffer* vertexBuffers[1] = { geometryHandle->Buffer.Get() };
			Context->IASetVertexBuffers(1, 1, vertexBuffers, &size, &offset);
		}
	}
}

void DirectX11Renderer::SetIndexBuffer(Geometry* geometry, std::size_t indexSize)
{
	if (geometry == nullptr)
	{
		Context->IASetIndexBuffer(nullptr, indexSize == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
	}
	else
	{
		auto geometryHandle = geometry->Handle<DirectX11GeometryHandle>();

		if (geometryHandle->Buffer)
			Context->IASetIndexBuffer(geometryHandle->Buffer.Get(), indexSize == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
	}
}

void DirectX11Renderer::SetConstantBuffer(Geometry* geometry, bool vertexAccess, bool fragmentAccess, int offset, std::size_t size, int slot)
{
	ID3D11Buffer* clear[1] = { nullptr };

	if (geometry == nullptr)
	{
		Context->VSSetConstantBuffers(slot, 1, clear);
		Context->PSSetConstantBuffers(slot, 1, clear);
	}
	else
	{
		auto geometryHandle = geometry->Handle<DirectX11GeometryHandle>();

		ID3D11Buffer* constantBuffers[1] = { geometryHandle->Buffer.Get() };

		ID3D11DeviceContext1* context;
		Context->QueryInterface<ID3D11DeviceContext1>(std::addressof(context));

		auto first = static_cast<UINT>(offset);
		auto count = static_cast<UINT>(((size + 255) >> 8) << 4);

		context->VSSetConstantBuffers1(slot, 1, vertexAccess ? constantBuffers : clear, &first, &count);
		context->PSSetConstantBuffers1(slot, 1, fragmentAccess ? constantBuffers : clear, &first, &count);
		context->Release();
	}
}

void DirectX11Renderer::DrawVertices(int firstVertex, int vertexCount)
{
	if (vertexCount > 0)
		Context->Draw(vertexCount, firstVertex);
}

void DirectX11Renderer::DrawIndices(int firstIndex, int indexCount)
{
	if (indexCount > 0)
		Context->DrawIndexed(indexCount, firstIndex, 0);
}

void DirectX11Renderer::DrawInstances(int firstVertex, int vertexCount, int firstInstance, int instanceCount)
{
	if (vertexCount > 0)
		Context->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
}

void DirectX11Renderer::DrawIndexedInstances(int firstIndex, int indexCount, int firstInstance, int instanceCount)
{
	if (indexCount > 0)
		Context->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, 0, firstInstance);
}

void DirectX11Renderer::EndFrame(int synchronization)
{
	SwapChain->Present(synchronization, 0);
}

auto DirectX11Renderer::GetAvailableSampleCounts() const -> List<int>
{
	List<int> counts;

	counts.Add(1);

	for (auto count = 2u; count <= 16; count *= 2)
	{
		UINT quality;
		Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, count, &quality);

		if (quality > 0)
			counts.Add(count);
	}

	return counts;
}

void DirectX11Renderer::CreateDevice(bool debug)
{
	CoInitialize(nullptr);

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_10_0 };
	auto flags = D3D11_CREATE_DEVICE_SINGLETHREADED | (debug ? D3D11_CREATE_DEVICE_DEBUG : 0);

	auto result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, flags, featureLevels, 1, D3D11_SDK_VERSION, &device, nullptr, &context);

	if (FAILED(result))
	{
		//Log::Write("failed to create the D3D11 device");
		return;
	}

	device.As(&Device);
	context.As(&Context);
}

void DirectX11Renderer::CreateSwapChain(IUnknown* window, HWND handle, int width, int height)
{
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	Context->OMSetRenderTargets(1, nullViews, nullptr);

	RenderTarget = nullptr;
	Context->Flush();

	if (SwapChain != nullptr)
	{
		auto result = SwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

		if (result != S_OK)
		{
			//Log::Write("failed to resize the swap chain");
			return;
		}
	}
	else
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDescription = { 0 };
		swapChainDescription.Width = width;
		swapChainDescription.Height = height;
		swapChainDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDescription.Stereo = false;
		swapChainDescription.SampleDesc.Count = 1;
		swapChainDescription.SampleDesc.Quality = 0;
		swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDescription.BufferCount = 2;
		swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		swapChainDescription.Flags = 0;
		swapChainDescription.Scaling = window == nullptr ? DXGI_SCALING_NONE : DXGI_SCALING_ASPECT_RATIO_STRETCH;
		swapChainDescription.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
		Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
		Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;

		Device.As(&dxgiDevice);
		dxgiDevice->GetAdapter(&adapter);
		adapter->GetParent(IID_PPV_ARGS(&factory));

		if (window == nullptr)
			factory->CreateSwapChainForHwnd(Device.Get(), handle, &swapChainDescription, nullptr, nullptr, &swapChain);
		else
			factory->CreateSwapChainForCoreWindow(Device.Get(), window, &swapChainDescription, nullptr, &swapChain);

		swapChain.As(&SwapChain);
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &RenderTarget);

	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f; 
	Viewport.Width = static_cast<float>(width);
	Viewport.Height = static_cast<float>(height);
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	D3D11_RECT rectangle;
	rectangle.left = 0;
	rectangle.top = 0;
	rectangle.right = width;
	rectangle.bottom = height;

	Context->RSSetViewports(1, &Viewport);
	Context->RSSetScissorRects(1, std::addressof(rectangle));
}