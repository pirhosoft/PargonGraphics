#pragma once

#include "Pargon/Application/Application.h"
#include "Pargon/Containers/Map.h"
#include "Pargon/Graphics/Geometry.h"
#include "Pargon/Graphics/GraphicsResource.h"
#include "Pargon/Graphics/Material.h"
#include "Pargon/Graphics/Renderer.h"
#include "Pargon/Graphics/Texture.h"
#include "Pargon/Types/Color.h"

#include <memory>
#include <mutex>

namespace Pargon
{
	class Color;

	class GraphicsDevice
	{
	public:
		static constexpr TextureId BackBuffer = {};
		static constexpr float DefaultDepthClear = 1.0f;
		static constexpr int DefaultStencilClear = 0;
		static constexpr float ClipNone = -1.0f;
		static constexpr int DrawAll = 0;
		static constexpr std::size_t ActualSize = 0;
		static constexpr int NoSynchronization = 0;
		static constexpr int VSync = 1;

		auto Renderer() const -> Renderer*;
		auto Geometries() const -> SequenceView<std::unique_ptr<Geometry>>;
		auto Materials() const -> SequenceView<std::unique_ptr<Material>>;
		auto Textures() const -> SequenceView<std::unique_ptr<Texture>>;

		auto Setup(Application& application, std::unique_ptr<Pargon::Renderer>&& renderer) -> RendererInformation;

		auto CreateGeometry(GraphicsStorage storage) -> Geometry*;
		auto CreateMaterial(GraphicsStorage storage) -> Material*;
		auto CreateTexture(GraphicsStorage storage) -> Texture*;
		auto GetGeometry(GeometryId id) -> Geometry*;
		auto GetMaterial(MaterialId id) -> Material*;
		auto GetTexture(TextureId id) -> Texture*;
		void DestroyGeometry(GeometryId id);
		void DestroyMaterial(MaterialId id);
		void DestroyTexture(TextureId id);

		void SetColorTarget(TextureId texture, int slot);
		void ClearColorTarget(float red, float green, float blue, float alpha);
		void SetDepthStencilTarget(TextureId texture);
		void ClearDepthStencilTarget(float depthValue, int stencilValue);
		void SetClippingRectangle(float x, float y, float width, float height);
		void SetMaterial(MaterialId material);
		void SetTexture(TextureId texture, int slot);
		void SetVertexBuffer(GeometryId geometry, std::size_t vertexSize);
		void SetInstanceBuffer(GeometryId geometry, std::size_t instanceSize);
		void SetIndexBuffer(GeometryId geometry, std::size_t indexSize);
		void SetConstantBuffer(GeometryId geometry, bool vertexAccess, bool fragmentAccess, int start, std::size_t size, int slot);
		void Draw(int start, int count);

		void Render(int synchronization);

	private:
		friend class GraphicsResource_;

		enum class RenderCommandType
		{
			SetColorTarget,
			SetDepthStencilTarget,
			ClearColorTarget,
			ClearDepthStencilTarget,
			SetClippingRectangle,
			SetMaterial,
			SetTexture,
			SetVertexBuffer,
			SetInstanceBuffer,
			SetIndexBuffer,
			SetConstantBuffer,
			Draw
		};

		struct RenderCommand
		{
			struct SetColorTarget
			{
				TextureId Texture;
				int Slot;
			};

			struct ClearColorTarget
			{
				float R;
				float G;
				float B;
				float A;
			};

			struct SetDepthStencilTarget
			{
				TextureId Texture;
			};

			struct ClearDepthStencilTarget
			{
				float DepthValue;
				int StencilValue;
			};

			struct SetClippingRectangle
			{
				float X;
				float Y;
				float Width;
				float Height;
			};

			struct SetMaterial
			{
				MaterialId Material;
			};

			struct SetTexture
			{
				TextureId Texture;
				int Slot;
			};

			struct SetVertexBuffer
			{
				GeometryId Geometry;
				std::size_t VertexSize;
			};

			struct SetInstanceBuffer
			{
				GeometryId Geometry;
				std::size_t InstanceSize;
			};

			struct SetIndexBuffer
			{
				GeometryId Geometry;
				std::size_t IndexSize;
			};

			struct SetConstantBuffer
			{
				GeometryId Geometry;
				int Start;
				std::size_t Size;
				int Slot;
				bool VertexAccess;
				bool FragmentAccess;
			};

			struct Draw
			{
				int Start;
				int Count;
			};

			union Data
			{
				Data() {}

				SetColorTarget SetColorTarget;
				ClearColorTarget ClearColorTarget;
				SetDepthStencilTarget SetDepthStencilTarget;
				ClearDepthStencilTarget ClearDepthStencilTarget;
				SetClippingRectangle SetClippingRectangle;
				SetMaterial SetMaterial;
				SetTexture SetTexture;
				SetVertexBuffer SetVertexBuffer;
				SetInstanceBuffer SetInstanceBuffer;
				SetIndexBuffer SetIndexBuffer;
				SetConstantBuffer SetConstantBuffer;
				Draw Draw;
			};

			RenderCommandType Type;
			Data Data;
		};

		std::unique_ptr<Pargon::Renderer> _renderer;
		std::mutex _resourceGuard;

		int _nextGeometryId = 0;
		int _nextMaterialId = 0;
		int _nextTextureId = 0;

		Map<int, std::unique_ptr<Geometry>> _geometries;
		Map<int, std::unique_ptr<Material>> _materials;
		Map<int, std::unique_ptr<Texture>> _textures;

		List<GraphicsResource_*> _pendingUpdates;
		List<RenderCommand> _commandQueue;

		int _vertexCount = 0;
		int _instanceCount = 0;
		int _indexCount = 0;

		void ExecuteCommand(const RenderCommand::SetColorTarget& command);
		void ExecuteCommand(const RenderCommand::ClearColorTarget& command);
		void ExecuteCommand(const RenderCommand::SetDepthStencilTarget& command);
		void ExecuteCommand(const RenderCommand::ClearDepthStencilTarget& command);
		void ExecuteCommand(const RenderCommand::SetClippingRectangle& command);
		void ExecuteCommand(const RenderCommand::SetMaterial& command);
		void ExecuteCommand(const RenderCommand::SetTexture& command);
		void ExecuteCommand(const RenderCommand::SetVertexBuffer& command);
		void ExecuteCommand(const RenderCommand::SetInstanceBuffer& command);
		void ExecuteCommand(const RenderCommand::SetIndexBuffer& command);
		void ExecuteCommand(const RenderCommand::SetConstantBuffer& command);
		void ExecuteCommand(const RenderCommand::Draw& command);
	};
}

inline
auto Pargon::GraphicsDevice::Renderer() const -> Pargon::Renderer*
{
	return _renderer.get();
}

inline
auto Pargon::GraphicsDevice::Geometries() const -> SequenceView<std::unique_ptr<Geometry>>
{
	return _geometries.Items();
}

inline
auto Pargon::GraphicsDevice::Materials() const -> SequenceView<std::unique_ptr<Material>>
{
	return _materials.Items();
}

inline
auto Pargon::GraphicsDevice::Textures() const -> SequenceView<std::unique_ptr<Texture>>
{
	return _textures.Items();
}
