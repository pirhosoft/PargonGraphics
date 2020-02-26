#include "Pargon/Application/Log.h"
#include "Pargon/Graphics/GraphicsDevice.h"
#include "DirectX11/DirectX11Geometry.h"
#include "DirectX11/DirectX11Renderer.h"

using namespace Pargon;

namespace
{
	auto GetBinding(GeometryTopology topology) -> UINT
	{
		switch (topology)
		{
		case GeometryTopology::TriangleList: return D3D11_BIND_VERTEX_BUFFER;
		case GeometryTopology::TriangleStrip: return D3D11_BIND_VERTEX_BUFFER;
		case GeometryTopology::PointList: return D3D11_BIND_VERTEX_BUFFER;
		case GeometryTopology::LineList: return D3D11_BIND_VERTEX_BUFFER;
		case GeometryTopology::IndexList: return D3D11_BIND_INDEX_BUFFER;
		case GeometryTopology::InstanceData: return D3D11_BIND_VERTEX_BUFFER;
		case GeometryTopology::ConstantData: return D3D11_BIND_CONSTANT_BUFFER;
		}

		return 0;
	}
}

auto DirectX11GeometryHandle::Update(Geometry* geometry) -> bool
{
	auto renderer = static_cast<DirectX11Renderer*>(geometry->Graphics().Renderer());

	auto requiredSize = static_cast<UINT>(geometry->Size());
	auto dynamic = geometry->Storage() == GraphicsStorage::StreamedToGpu;
	auto binding = GetBinding(geometry->Topology());

	if (!Buffer || !dynamic || requiredSize > Capacity || binding != Binding)
	{
		Buffer.Reset();
		Capacity = requiredSize;
		Binding = binding;

		if (Capacity > 0)
		{
			D3D11_BUFFER_DESC description;
			description.ByteWidth = Capacity;
			description.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
			description.BindFlags = binding;
			description.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
			description.MiscFlags = 0;
			description.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA resource;
			D3D11_SUBRESOURCE_DATA* resourceData = nullptr;

			if (requiredSize > 0)
			{
				auto vertices = geometry->Data().begin();

				resource.pSysMem = vertices;
				resource.SysMemPitch = 0;
				resource.SysMemSlicePitch = 0;
				resourceData = &resource;
			}

			auto result = renderer->Device->CreateBuffer(&description, resourceData, Buffer.GetAddressOf());
			if (FAILED(result))
				return false;
		}
	}
	else
	{
		if (requiredSize > 0)
		{
			D3D11_MAPPED_SUBRESOURCE resource;
			ZeroMemory(&resource, sizeof(D3D11_MAPPED_SUBRESOURCE));

			renderer->Context->Map(Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
			std::copy(geometry->Data().begin(), geometry->Data().end(), reinterpret_cast<char*>(resource.pData));
			renderer->Context->Unmap(Buffer.Get(), 0);
		}
	}

	return true;
}