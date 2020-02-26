#pragma once

#include "Pargon/Graphics/Renderer.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <d3d11.h>
#include <wrl.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace Pargon
{
	class DirectX11Renderer : public Renderer
	{
	public:
		~DirectX11Renderer();

		Microsoft::WRL::ComPtr<ID3D11Device> Device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context;
		Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTarget;

		D3D11_VIEWPORT Viewport;

		auto HorizontalResolution() -> unsigned int override;
		auto VerticalResolution() -> unsigned int override;

		auto CompileShader(StringView content, Material& material) -> ShaderCompilationResult override;

	protected:
		void Setup(Application& application, RendererInformation& information) override;

		auto CreateGeometryHandle() -> std::unique_ptr<GeometryHandle> override;
		auto CreateMaterialHandle() -> std::unique_ptr<MaterialHandle> override;
		auto CreateTextureHandle() -> std::unique_ptr<TextureHandle> override;

		void BeginFrame() override;
		void SetRenderTarget(Texture* texture, int slot) override;
		void ClearColorTarget(float r, float g, float b, float a) override;
		void SetDepthStencilTarget(Texture* texture) override;
		void ClearDepthAndStencilTarget(float depth, int stencil) override;
		void SetClippingRectangle(float x, float y, float width, float height) override;
		void SetMaterial(Material* material) override;
		void SetTexture(Texture* texture, int slot) override;
		void SetVertexBuffer(Geometry* geometry, std::size_t vertexSize) override;
		void SetInstanceBuffer(Geometry* geometry, std::size_t vertexSize) override;
		void SetIndexBuffer(Geometry* geometry, std::size_t indexSize) override;
		void SetConstantBuffer(Geometry* geometry, bool vertexAccess, bool fragmentAccess, int offset, std::size_t size, int slot) override;
		void DrawVertices(int firstVertex, int vertexCount) override;
		void DrawIndices(int firstIndex, int indexCount) override;
		void DrawInstances(int firstVertex, int vertexCount, int firstInstance, int instanceCount) override;
		void DrawIndexedInstances(int firstIndex, int indexCount, int firstInstance, int instanceCount) override;
		void EndFrame(int synchronization) override;

	private:
		List<ID3D11RenderTargetView*> _currentRenderTargets;
		ID3D11DepthStencilView* _currentDepthStencilTarget = nullptr;
		D3D11_VIEWPORT _currentViewport;
		int _frameSynchronization;

		auto GetAvailableSampleCounts() const -> List<int>;

		void CreateDevice(bool debug);
		void CreateSwapChain(IUnknown* window, HWND handle, int width, int height);
	};
}

inline
auto Pargon::DirectX11Renderer::HorizontalResolution() -> unsigned int
{
	return static_cast<unsigned int>(Viewport.Width);
}

inline
auto Pargon::DirectX11Renderer::VerticalResolution() -> unsigned int
{
	return static_cast<unsigned int>(Viewport.Height);
}
