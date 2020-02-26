#include "Pargon/Application/Log.h"
#include "Pargon/Graphics/GraphicsDevice.h"
#include "Pargon/Types/Color.h"

using namespace Pargon;

auto GraphicsDevice::Setup(Application& application, std::unique_ptr<Pargon::Renderer>&& renderer) -> RendererInformation
{
	_renderer = std::move(renderer);

	RendererInformation information;

	if (_renderer == nullptr)
	{
		information.Available = false;
		information.Messages.Add(FormatString("could not create renderer"));
	}
	else
	{
		_renderer->Setup(application, information);
	}

	return information;
}

auto GraphicsDevice::CreateGeometry(GraphicsStorage storage) -> Geometry*
{
	std::lock_guard<std::mutex> lock(_resourceGuard);

	auto id = _nextGeometryId++;

	assert(_renderer);
	assert(_geometries.GetIndex(id) == Sequence::InvalidIndex);

	return _geometries.AddOrSet(id, std::unique_ptr<Geometry>(new Geometry(*this, storage, _renderer->CreateGeometryHandle(), id))).get();
}

void GraphicsDevice::DestroyGeometry(GeometryId id)
{
	std::lock_guard<std::mutex> lock(_resourceGuard);

	assert(_renderer);
	assert(_geometries.GetIndex(id._id) != Sequence::InvalidIndex);
	assert(!GetGeometry(id)->IsLocked());
	assert(!GetGeometry(id)->IsChanged());

	_geometries.RemoveWithKey(id._id);
}

auto GraphicsDevice::GetGeometry(GeometryId id) -> Geometry*
{
	auto index = _geometries.GetIndex(id._id);
	return index == Sequence::InvalidIndex ? nullptr : _geometries.ItemAtIndex(index).get();
}

auto GraphicsDevice::CreateMaterial(GraphicsStorage storage) -> Material*
{
	std::lock_guard<std::mutex> lock(_resourceGuard);

	auto id = _nextMaterialId++;

	assert(_renderer);
	assert(_materials.GetIndex(id) == Sequence::InvalidIndex);

	return _materials.AddOrSet(id, std::unique_ptr<Material>(new Material(*this, storage, _renderer->CreateMaterialHandle(), id))).get();
}

void GraphicsDevice::DestroyMaterial(MaterialId id)
{
	std::lock_guard<std::mutex> lock(_resourceGuard);

	assert(_renderer);
	assert(_materials.GetIndex(id._id) != Sequence::InvalidIndex);
	assert(!GetMaterial(id)->IsLocked());
	assert(!GetMaterial(id)->IsChanged());

	_materials.RemoveWithKey(id._id);
}

auto GraphicsDevice::GetMaterial(MaterialId id) -> Material*
{
	auto index = _materials.GetIndex(id._id);
	return index == Sequence::InvalidIndex ? nullptr : _materials.ItemAtIndex(index).get();
}

auto GraphicsDevice::CreateTexture(GraphicsStorage storage) -> Texture*
{
	std::lock_guard<std::mutex> lock(_resourceGuard);

	auto id = _nextTextureId++;

	assert(_renderer);
	assert(_textures.GetIndex(id) == Sequence::InvalidIndex);

	return _textures.AddOrSet(id, std::unique_ptr<Texture>(new Texture(*this, storage, _renderer->CreateTextureHandle(), id))).get();
}

void GraphicsDevice::DestroyTexture(TextureId id)
{
	std::lock_guard<std::mutex> lock(_resourceGuard);

	assert(_renderer);
	assert(_textures.GetIndex(id._id) != Sequence::InvalidIndex);
	assert(!GetTexture(id)->IsLocked());
	assert(!GetTexture(id)->IsChanged());

	_textures.RemoveWithKey(id._id);
}

auto GraphicsDevice::GetTexture(TextureId id) -> Texture*
{
	auto index = _textures.GetIndex(id._id);
	return index == Sequence::InvalidIndex ? nullptr : _textures.ItemAtIndex(index).get();
}

void GraphicsDevice::SetColorTarget(TextureId texture, int slot)
{
	auto& command = _commandQueue.Increment();
	command.Type = RenderCommandType::SetColorTarget;
	command.Data.SetColorTarget.Texture = texture;
	command.Data.SetColorTarget.Slot = slot;
}

void GraphicsDevice::ClearColorTarget(float red, float green, float blue, float alpha)
{
	auto& command = _commandQueue.Increment();
	command.Type = RenderCommandType::ClearColorTarget;
	command.Data.ClearColorTarget.R = red;
	command.Data.ClearColorTarget.G = green;
	command.Data.ClearColorTarget.B = blue;
	command.Data.ClearColorTarget.A = alpha;
}

void GraphicsDevice::SetDepthStencilTarget(TextureId texture)
{
	auto& command = _commandQueue.Increment();
	command.Type = RenderCommandType::SetDepthStencilTarget;
	command.Data.SetDepthStencilTarget.Texture = texture;
}

void GraphicsDevice::ClearDepthStencilTarget(float depthValue, int stencilValue)
{
	auto& command = _commandQueue.Increment();
	command.Type = RenderCommandType::ClearDepthStencilTarget;
	command.Data.ClearDepthStencilTarget.DepthValue = depthValue;
	command.Data.ClearDepthStencilTarget.StencilValue = stencilValue;
}

void GraphicsDevice::SetClippingRectangle(float x, float y, float width, float height)
{
	auto& command = _commandQueue.Increment();
	command.Type = RenderCommandType::SetClippingRectangle;
	command.Data.SetClippingRectangle.X = x;
	command.Data.SetClippingRectangle.Y = y;
	command.Data.SetClippingRectangle.Width = width;
	command.Data.SetClippingRectangle.Height = height;
}

void GraphicsDevice::SetMaterial(MaterialId material)
{
	auto& command = _commandQueue.Increment();
	command.Type = RenderCommandType::SetMaterial;
	command.Data.SetMaterial.Material = material;
}

void GraphicsDevice::SetTexture(TextureId texture, int slot)
{
	auto& command = _commandQueue.Increment();
	command.Type = RenderCommandType::SetTexture;
	command.Data.SetTexture.Texture = texture;
	command.Data.SetTexture.Slot = slot;
}

void GraphicsDevice::SetVertexBuffer(GeometryId geometry, std::size_t vertexSize)
{
	auto& command = _commandQueue.Increment();
	command.Type = RenderCommandType::SetVertexBuffer;
	command.Data.SetVertexBuffer.Geometry = geometry;
	command.Data.SetVertexBuffer.VertexSize = vertexSize;
}

void GraphicsDevice::SetInstanceBuffer(GeometryId geometry, std::size_t instanceSize)
{
	auto& command = _commandQueue.Increment();
	command.Type = RenderCommandType::SetInstanceBuffer;
	command.Data.SetInstanceBuffer.Geometry = geometry;
	command.Data.SetInstanceBuffer.InstanceSize = instanceSize;
}

void GraphicsDevice::SetIndexBuffer(GeometryId geometry, std::size_t indexSize)
{
	auto& command = _commandQueue.Increment();
	command.Type = RenderCommandType::SetIndexBuffer;
	command.Data.SetIndexBuffer.Geometry = geometry;
	command.Data.SetIndexBuffer.IndexSize = indexSize;
}

void GraphicsDevice::SetConstantBuffer(GeometryId geometry, bool vertexAccess, bool fragmentAccess, int start, std::size_t size, int slot)
{
	auto& command = _commandQueue.Increment();
	command.Type = RenderCommandType::SetConstantBuffer;
	command.Data.SetConstantBuffer.Geometry = geometry;
	command.Data.SetConstantBuffer.Start = start;
	command.Data.SetConstantBuffer.Size = size;
	command.Data.SetConstantBuffer.Slot = slot;
	command.Data.SetConstantBuffer.VertexAccess = vertexAccess;
	command.Data.SetConstantBuffer.FragmentAccess = fragmentAccess;
}

void GraphicsDevice::Draw(int start, int count)
{
	auto& command = _commandQueue.Increment();
	command.Type = RenderCommandType::Draw;
	command.Data.Draw.Start = start;
	command.Data.Draw.Count = count;
}

void GraphicsDevice::Render(int synchronization)
{
	assert(_renderer);

	std::lock_guard<std::mutex> lock(_resourceGuard);

	for (auto resource : _pendingUpdates)
	{
		if (!resource->_isLocked)
			resource->_handle->Update(resource);
	}

	_renderer->BeginFrame();

	for (auto& command : _commandQueue)
	{
		switch (command.Type)
		{
		case RenderCommandType::SetColorTarget: ExecuteCommand(command.Data.SetColorTarget); break;
		case RenderCommandType::ClearColorTarget: ExecuteCommand(command.Data.ClearColorTarget); break;
		case RenderCommandType::SetDepthStencilTarget: ExecuteCommand(command.Data.SetDepthStencilTarget); break;
		case RenderCommandType::ClearDepthStencilTarget: ExecuteCommand(command.Data.ClearDepthStencilTarget); break;
		case RenderCommandType::SetClippingRectangle: ExecuteCommand(command.Data.SetClippingRectangle); break;
		case RenderCommandType::SetMaterial: ExecuteCommand(command.Data.SetMaterial); break;
		case RenderCommandType::SetTexture: ExecuteCommand(command.Data.SetTexture); break;
		case RenderCommandType::SetVertexBuffer: ExecuteCommand(command.Data.SetVertexBuffer); break;
		case RenderCommandType::SetInstanceBuffer: ExecuteCommand(command.Data.SetInstanceBuffer); break;
		case RenderCommandType::SetIndexBuffer: ExecuteCommand(command.Data.SetIndexBuffer); break;
		case RenderCommandType::SetConstantBuffer: ExecuteCommand(command.Data.SetConstantBuffer); break;
		case RenderCommandType::Draw: ExecuteCommand(command.Data.Draw); break;
		}
	}

	_renderer->EndFrame(synchronization);

	_vertexCount = 0;
	_indexCount = 0;
	_instanceCount = 0;

	_commandQueue.Clear();

	for (auto resource : _pendingUpdates)
	{
		if (!resource->_isLocked)
			resource->UpdateComplete();
	}

	_pendingUpdates.Clear();
}

void GraphicsDevice::ExecuteCommand(const RenderCommand::SetColorTarget& command)
{
	auto texture = GetTexture(command.Texture);
	_renderer->SetRenderTarget(texture, command.Slot);
}

void GraphicsDevice::ExecuteCommand(const RenderCommand::ClearColorTarget& command)
{
	_renderer->ClearColorTarget(command.R, command.G, command.B, command.A);
}

void GraphicsDevice::ExecuteCommand(const RenderCommand::SetDepthStencilTarget& command)
{
	auto texture = GetTexture(command.Texture);
	_renderer->SetDepthStencilTarget(texture);
}

void GraphicsDevice::ExecuteCommand(const RenderCommand::ClearDepthStencilTarget& command)
{
	_renderer->ClearDepthAndStencilTarget(command.DepthValue, command.StencilValue);
}

void GraphicsDevice::ExecuteCommand(const RenderCommand::SetClippingRectangle& command)
{
	_renderer->SetClippingRectangle(command.X, command.Y, command.Width, command.Height);
}

void GraphicsDevice::ExecuteCommand(const RenderCommand::SetMaterial& command)
{
	auto material = GetMaterial(command.Material);
	_renderer->SetMaterial(material);
}

void GraphicsDevice::ExecuteCommand(const RenderCommand::SetTexture& command)
{
	auto texture = GetTexture(command.Texture);
	_renderer->SetTexture(texture, command.Slot);
}

void GraphicsDevice::ExecuteCommand(const RenderCommand::SetVertexBuffer& command)
{
	auto geometry = GetGeometry(command.Geometry);
	auto size = command.VertexSize;

	_vertexCount = geometry == nullptr || size == 0 ? 0 : static_cast<int>(geometry->Size() / size);
	_indexCount = 0;
	_instanceCount = 0;

	_renderer->SetVertexBuffer(geometry, size);
}

void GraphicsDevice::ExecuteCommand(const RenderCommand::SetInstanceBuffer& command)
{
	auto geometry = GetGeometry(command.Geometry);
	auto size = command.InstanceSize;

	_instanceCount = geometry == nullptr || size == 0 ? 0 : static_cast<int>(geometry->Size() / size);

	_renderer->SetInstanceBuffer(geometry, size);
}

void GraphicsDevice::ExecuteCommand(const RenderCommand::SetIndexBuffer& command)
{
	auto geometry = GetGeometry(command.Geometry);
	auto size = command.IndexSize;

	_indexCount = geometry == nullptr || size == 0 ? 0 : static_cast<int>(geometry->Size() / size);

	_renderer->SetIndexBuffer(geometry, size);
}

void GraphicsDevice::ExecuteCommand(const RenderCommand::SetConstantBuffer& command)
{
	auto geometry = GetGeometry(command.Geometry);

	_renderer->SetConstantBuffer(geometry, command.VertexAccess, command.FragmentAccess, command.Start, command.Size, command.Slot);
}

void GraphicsDevice::ExecuteCommand(const RenderCommand::Draw& command)
{
	if (_instanceCount > 0)
	{
		if (_indexCount > 0)
			_renderer->DrawIndexedInstances(0, _indexCount, command.Start, command.Count == 0 ? _instanceCount : command.Count);
		else
			_renderer->DrawInstances(0, _vertexCount, command.Start, command.Count == 0 ? _instanceCount : command.Count);
	}
	else
	{
		if (_indexCount > 0)
			_renderer->DrawIndices(command.Start, command.Count == 0 ? _indexCount : command.Count);
		else
			_renderer->DrawVertices(command.Start, command.Count == 0 ? _vertexCount : command.Count);
	}
}