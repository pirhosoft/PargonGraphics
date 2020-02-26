#pragma once

#include "Pargon/Application/Application.h"
#include "Pargon/Graphics/Geometry.h"
#include "Pargon/Graphics/Material.h"
#include "Pargon/Graphics/Texture.h"

namespace Pargon
{
	class Log;

	struct RendererInformation
	{
		String Type;
		bool Available;
		List<String> Messages;

		List<int> SupportedSampleCounts;

		void WriteInformation(Log& log);
		void WriteCapabilities(Log& log);
	};

	class Renderer
	{
	public:
		virtual ~Renderer() = default;

		virtual auto HorizontalResolution() -> unsigned int = 0;
		virtual auto VerticalResolution() -> unsigned int = 0;

		virtual auto CompileShader(StringView content, Material& material) -> ShaderCompilationResult = 0;

	protected:
		friend class GraphicsDevice;

		virtual void Setup(Application& application, RendererInformation& information) = 0;

		virtual auto CreateGeometryHandle() -> std::unique_ptr<GeometryHandle> = 0;
		virtual auto CreateMaterialHandle() -> std::unique_ptr<MaterialHandle> = 0;
		virtual auto CreateTextureHandle() -> std::unique_ptr<TextureHandle> = 0;

		virtual void BeginFrame() = 0;
		virtual void SetRenderTarget(Texture* texture, int slot) = 0;
		virtual void ClearColorTarget(float r, float g, float b, float a) = 0;
		virtual void SetDepthStencilTarget(Texture* texture) = 0;
		virtual void ClearDepthAndStencilTarget(float depth, int stencil) = 0;
		virtual void SetClippingRectangle(float x, float y, float width, float height) = 0;
		virtual void SetMaterial(Material* material) = 0;
		virtual void SetTexture(Texture* texture, int slot) = 0;
		virtual void SetVertexBuffer(Geometry* geometry, std::size_t vertexSize) = 0;
		virtual void SetInstanceBuffer(Geometry* geometry, std::size_t vertexSize) = 0;
		virtual void SetIndexBuffer(Geometry* geometry, std::size_t indexSize) = 0;
		virtual void SetConstantBuffer(Geometry* geometry, bool vertexAccess, bool fragmentAccess, int offset, std::size_t size, int slot) = 0;
		virtual void DrawVertices(int firstVertex, int vertexCount) = 0;
		virtual void DrawIndices(int firstIndex, int indexCount) = 0;
		virtual void DrawInstances(int firstVertex, int vertexCount, int firstInstance, int instanceCount) = 0;
		virtual void DrawIndexedInstances(int firstIndex, int indexCount, int firstInstance, int instanceCount) = 0;
		virtual void EndFrame(int synchronization) = 0;
	};
}
